// Copyright Druid Mechanics

#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/Core/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Character/RPGCharacter.h"
#include "Interaction/PlayerInterface.h"

// Adiciona as habilidades iniciais ao componente
void URPGAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
    for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
    {
        FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
        if (const URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(AbilitySpec.Ability))
        {
            AbilitySpec.GetDynamicSpecSourceTags().AddTag(RPGAbility->StartupInputTag);
            GiveAbility(AbilitySpec);
        }
    }
}

void URPGAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
    for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
    {
        FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
        GiveAbilityAndActivateOnce(AbilitySpec);
    }
}

void URPGAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
	
	// Enviar evento usando a tag criada
	if (GetAvatarActor())
	{
		FGameplayEventData Payload;
		Payload.EventTag = FRPGGameplayTags::Get().Events_Abilities_InputPressed;
		Payload.Instigator = GetAvatarActor();
		// Adicionar o InputTag no InstigatorTags para que as abilities possam filtrar qual input foi pressionado
		Payload.InstigatorTags.AddTag(InputTag);
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), FRPGGameplayTags::Get().Events_Abilities_InputPressed, Payload);
	}
}

void URPGAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
	
	// Enviar evento usando a tag criada
	if (GetAvatarActor())
	{
		FGameplayEventData Payload;
		Payload.EventTag = FRPGGameplayTags::Get().Events_Abilities_InputHeld;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), FRPGGameplayTags::Get().Events_Abilities_InputHeld, Payload);
	}
}

void URPGAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
	
	// Enviar evento usando a tag criada
	if (GetAvatarActor())
	{
		FGameplayEventData Payload;
		Payload.EventTag = FRPGGameplayTags::Get().Events_Abilities_InputReleased;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), FRPGGameplayTags::Get().Events_Abilities_InputReleased, Payload);
	}
}

// Notifica que o AbilityActorInfo foi inicializado (stub)
void URPGAbilitySystemComponent::AbilityActorInfoSet()
{
    // Implementar se necessário
}

void URPGAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
    if (GetAvatarActor()->Implements<UPlayerInterface>())
    {
        if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
        {
            ServerUpgradeAttribute(AttributeTag);
        }
    }
}

bool URPGAbilitySystemComponent::ServerUpgradeAttribute_Validate(const FGameplayTag& AttributeTag)
{
    return true;
}

void URPGAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
    FGameplayEventData Payload;
    Payload.EventTag = AttributeTag;
    Payload.EventMagnitude = 1.f;

    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

    if (GetAvatarActor()->Implements<UPlayerInterface>())
    {
        IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
    }
}

// Retorna o actor possuído por este ASC
AActor* URPGAbilitySystemComponent::GetAvatarActor() const
{
    return GetOwner();
}

bool URPGAbilitySystemComponent::HasActiveAbilityWithInputTag(const FGameplayTag& InputTag) const
{
    if (!InputTag.IsValid()) return false;
    for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.IsActive() && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
        {
            return true;
        }
    }
    return false;
}

// === ABILITY LEVEL MANAGEMENT ===

void URPGAbilitySystemComponent::SetAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
    // Se não tem autoridade, chamar RPC para o servidor
    if (!IsValid(GetAvatarActor()) || !GetAvatarActor()->HasAuthority())
    {
        ServerSetAbilityLevel(AbilityClass, Level);
        return;
    }

    // Executar no servidor
    if (!AbilityClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SetAbilityLevel] AbilityClass é nullptr"));
        return;
    }

    FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromClass(AbilityClass);
    if (!AbilitySpec)
    {
        UE_LOG(LogTemp, Warning, TEXT("[SetAbilityLevel] Habilidade não encontrada: %s"), *GetNameSafe(AbilityClass));
        return;
    }

    int32 OldLevel = AbilitySpec->Level;
    AbilitySpec->Level = Level;
    MarkAbilitySpecDirty(*AbilitySpec);
    
    UE_LOG(LogTemp, Log, TEXT("[SetAbilityLevel] Habilidade %s: Nível %d -> %d"), 
        *GetNameSafe(AbilityClass), OldLevel, Level);
}

void URPGAbilitySystemComponent::AddToAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
    // Se não tem autoridade, chamar RPC para o servidor
    if (!IsValid(GetAvatarActor()) || !GetAvatarActor()->HasAuthority())
    {
        ServerAddToAbilityLevel(AbilityClass, Level);
        return;
    }

    // Executar no servidor
    if (!AbilityClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AddToAbilityLevel] AbilityClass é nullptr"));
        return;
    }

    FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromClass(AbilityClass);
    if (!AbilitySpec)
    {
        UE_LOG(LogTemp, Warning, TEXT("[AddToAbilityLevel] Habilidade não encontrada: %s"), *GetNameSafe(AbilityClass));
        return;
    }

    int32 OldLevel = AbilitySpec->Level;
    AbilitySpec->Level += Level;
    MarkAbilitySpecDirty(*AbilitySpec);
    
    UE_LOG(LogTemp, Log, TEXT("[AddToAbilityLevel] Habilidade %s: Nível %d -> %d (+%d)"), 
        *GetNameSafe(AbilityClass), OldLevel, AbilitySpec->Level, Level);
}

// === RPC IMPLEMENTATIONS ===

void URPGAbilitySystemComponent::ServerSetAbilityLevel_Implementation(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
    SetAbilityLevel(AbilityClass, Level);
}

void URPGAbilitySystemComponent::ServerAddToAbilityLevel_Implementation(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level)
{
    AddToAbilityLevel(AbilityClass, Level);
}

// === OVERRIDES ===

void URPGAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
    Super::OnGiveAbility(AbilitySpec);
    HandleAutoActivateAbility(AbilitySpec);
}

void URPGAbilitySystemComponent::OnRep_ActivateAbilities()
{
    Super::OnRep_ActivateAbilities();
    
    FScopedAbilityListLock ActiveScopeLock(*this);
    for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        HandleAutoActivateAbility(AbilitySpec);
    }
}

void URPGAbilitySystemComponent::HandleAutoActivateAbility(const FGameplayAbilitySpec& AbilitySpec)
{
    if (!IsValid(AbilitySpec.Ability)) return;
    
    for (const FGameplayTag& Tag : AbilitySpec.Ability->GetAssetTags())
    {
        if (Tag.MatchesTagExact(FRPGGameplayTags::Get().Abilities_ActivateOnGiven))
        {
            TryActivateAbility(AbilitySpec.Handle);
            return;
        }
    }
}
