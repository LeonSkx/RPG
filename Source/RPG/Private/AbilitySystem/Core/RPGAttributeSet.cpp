#include "AbilitySystem/Core/RPGAttributeSet.h"

// Ability System
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Core/RPGAbilitySystemLibrary.h"

// Core & Engine
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

// Interfaces
#include "Interaction/PlayerInterface.h"
#include "Interaction/CombatInterface.h"

// Characters
#include "Character/RPGCharacter.h"
#include "Character/RPGEnemy.h"

// Player
#include "Player/RPGPlayerController.h"

// AI & Perception
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Damage.h"

// Quests
#include "Quest/QuestSubsystem.h"


/* =============================
 *  Constructor
 * ============================= */
URPGAttributeSet::URPGAttributeSet()
{
}


/* =============================
 *  Replication Setup
 * ============================= */
void URPGAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Primary Attributes
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Strength, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

    // Secondary Attributes - Removidos para simplificar o sistema

    // Core Combat Attributes
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Attack, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MagicDamage, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Accuracy, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Armor, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MagicResistance, COND_None, REPNOTIFY_Always);

    // Vital Attributes
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(URPGAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
    // Energy removido - Mana já serve para recursos
}


/* =============================
 *  Gameplay Effect Handling
 * ============================= */
void URPGAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    FEffectProperties Props;
    SetEffectProperties(Data, Props);

    // Armazenar Health antes de qualquer modificação para verificar morte
    const float HealthBeforeModification = GetHealth();

    // Clamp Health
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
        
        // Verificar se Health chegou a 0 ou menos (morte) quando modificado diretamente
        // Só verificar se não foi através de IncomingDamage (que já trata morte no bloco abaixo)
        const float HealthAfterClamp = GetHealth();
        if (HealthAfterClamp <= 0.f && HealthBeforeModification > 0.f)
        {
            // Verificar se o dano não veio de IncomingDamage (para evitar duplicação)
            // Se o atributo modificado foi Health diretamente, verificar morte
            if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor))
            {
                // Verificar se já está morto para evitar múltiplas chamadas
                if (!CombatInterface->IsDead())
                {
                    CombatInterface->Die(FVector::ZeroVector);
                    SendXPEvent(Props);
                }
            }
        }
    }

    // Clamp Mana
    if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
    }

    // Handle Incoming Damage
    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        const float LocalIncomingDamage = GetIncomingDamage();
        SetIncomingDamage(0.f);

        if (LocalIncomingDamage > 0.f)
        {
            const float NewHealth = GetHealth() - LocalIncomingDamage;
            SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

            // Report damage to AI system
            if (Props.SourceAvatarActor)
            {
                FAIDamageEvent DamageEvent(
                    Props.TargetAvatarActor,
                    Props.SourceAvatarActor,
                    LocalIncomingDamage,
                    Props.TargetAvatarActor->GetActorLocation()
                );
                UAIPerceptionSystem::OnEvent(Props.TargetAvatarActor->GetWorld(), DamageEvent);
            }

            const bool bFatal = NewHealth <= 0.f;
            if (bFatal)
            {
                if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor))
                {
                    CombatInterface->Die(FVector::ZeroVector);
                }
                SendXPEvent(Props);
            }
            else
            {
                // Trigger Hit React
                FGameplayTagContainer TagContainer;
                TagContainer.AddTag(FRPGGameplayTags::Get().Effects_HitReact);
                Props.TargetASC->TryActivateAbilitiesByTag(TagContainer);
            }

            // Extra checks (block, crit)
            const bool bBlock = URPGAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
            const bool bCriticalHit = URPGAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
        }
    }

    // Handle Incoming XP
    if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
    {
        const float LocalIncomingXP = GetIncomingXP();
        SetIncomingXP(0.f);

        if (LocalIncomingXP <= 0.f || !Props.SourceCharacter) return;

        if (!Props.SourceCharacter->Implements<UPlayerInterface>() ||
            !Props.SourceCharacter->Implements<UCombatInterface>())
        {
            return;
        }

        const int32 CurrentLevel = ICombatInterface::Execute_GetCharacterLevel(Props.SourceCharacter);
        const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);

        IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
    }
}


/* =============================
 *  Helper: Effect Properties
 * ============================= */
void URPGAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const
{
    Props.EffectContextHandle = Data.EffectSpec.GetContext();
    Props.SourceASC = Props.EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();

    if (IsValid(Props.SourceASC) &&
        Props.SourceASC->AbilityActorInfo.IsValid() &&
        Props.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
    {
        Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
        Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();

        if (!Props.SourceController && Props.SourceAvatarActor)
        {
            if (const APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
            {
                Props.SourceController = Pawn->GetController();
            }
        }

        if (Props.SourceController)
        {
            Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
        }
    }

    if (Data.Target.AbilityActorInfo.IsValid() &&
        Data.Target.AbilityActorInfo->AvatarActor.IsValid())
    {
        Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
        Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
        Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
        Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
    }
}


/* =============================
 *  Replication Notifies
 * ============================= */

// Vital Attributes
void URPGAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue) const       { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Health, OldValue); }
void URPGAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const    { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxHealth, OldValue); }
void URPGAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldValue) const         { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Mana, OldValue); }
void URPGAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue) const      { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MaxMana, OldValue); }
// Energy removido - Mana já serve para recursos

// Primary Attributes
void URPGAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldValue) const     { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Strength, OldValue); }
void URPGAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldValue) const { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Intelligence, OldValue); }
void URPGAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldValue) const   { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Resilience, OldValue); }
void URPGAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldValue) const        { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Vigor, OldValue); }

// Secondary Attributes - Removidos para simplificar o sistema

// Core Combat
void URPGAttributeSet::OnRep_Attack(const FGameplayAttributeData& OldValue) const               { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Attack, OldValue); }
void URPGAttributeSet::OnRep_MagicDamage(const FGameplayAttributeData& OldValue) const          { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MagicDamage, OldValue); }
void URPGAttributeSet::OnRep_Accuracy(const FGameplayAttributeData& OldValue) const             { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Accuracy, OldValue); }
void URPGAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldValue) const                { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, Armor, OldValue); }
void URPGAttributeSet::OnRep_MagicResistance(const FGameplayAttributeData& OldValue) const      { GAMEPLAYATTRIBUTE_REPNOTIFY(URPGAttributeSet, MagicResistance, OldValue); }


/* =============================
 *  XP & Quest Events
 * ============================= */
void URPGAttributeSet::SendXPEvent(const FEffectProperties& Props)
{
    // Grant XP only if target is an enemy
    if (ARPGEnemy* Enemy = Cast<ARPGEnemy>(Props.TargetCharacter))
    {
        const int32 TargetLevel = Enemy->GetCharacterLevel_Implementation();
        const ECharacterClass TargetClass = Enemy->GetCharacterClass_Implementation();
        const int32 XPReward = URPGAbilitySystemLibrary::GetXPRewardForClassAndLevel(
            Props.TargetCharacter, TargetClass, TargetLevel);

        const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
        FGameplayEventData Payload;
        Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;
        Payload.EventMagnitude = XPReward;

        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
            Props.SourceCharacter, GameplayTags.Attributes_Meta_IncomingXP, Payload);
    }

    // Update Quest Progress
    if (ARPGEnemy* DeadEnemy = Cast<ARPGEnemy>(Props.TargetCharacter))
    {
        if (ARPGCharacter* Player = Cast<ARPGCharacter>(Props.SourceCharacter))
        {
            if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
            {
                if (UQuestSubsystem* QuestSystem = GameInstance->GetSubsystem<UQuestSubsystem>())
                {
                    FString EnemyType = DeadEnemy->EnemyType;
                    QuestSystem->UpdateKillProgressAuto(EnemyType, 1, Player);
                }
            }
        }
    }
}
