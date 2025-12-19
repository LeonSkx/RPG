// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"
#include "GameFramework/PlayerController.h"
#include "AI/RPGPartyAIController.h"
#include "Engine/Engine.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"

void URPGGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo,
                                         const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
    if (IsPlayerController())
    {
        OnPlayerAbilityActivated(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    }
    else if (IsPartyAIController())
    {
        OnPartyAIAbilityActivated(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    }
    else
    {
        // Lógica padrão, se necessário
        Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
        
        if (bDrawDebug && IsValid(GEngine))
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::Printf(TEXT("%s Activated"), *GetName()));
        }
    }
}

void URPGGameplayAbility::OnPlayerAbilityActivated(const FGameplayAbilitySpecHandle Handle,
                                                  const FGameplayAbilityActorInfo* ActorInfo,
                                                  const FGameplayAbilityActivationInfo ActivationInfo,
                                                  const FGameplayEventData* TriggerEventData)
{
    // Lógica padrão do player (pode ser sobrescrita nas filhas)
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    if (bDrawDebug && IsValid(GEngine))
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::Printf(TEXT("%s Activated"), *GetName()));
    }
}

void URPGGameplayAbility::OnPartyAIAbilityActivated(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
    // Lógica padrão do AI da party (pode ser sobrescrita nas filhas)
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    if (bDrawDebug && IsValid(GEngine))
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::Printf(TEXT("%s Activated"), *GetName()));
    }
}

bool URPGGameplayAbility::IsValidController() const
{
    if (!GetOwningActorFromActorInfo())
    {
        return false;
    }

    AController* Controller = GetOwningActorFromActorInfo()->GetInstigatorController();
    return IsValid(Controller);
}

bool URPGGameplayAbility::IsPlayerController() const
{
    if (!IsValidController())
    {
        return false;
    }

    AController* Controller = GetOwningActorFromActorInfo()->GetInstigatorController();
    return Cast<APlayerController>(Controller) != nullptr;
}

bool URPGGameplayAbility::IsPartyAIController() const
{
    if (!IsValidController())
    {
        return false;
    }

    AController* Controller = GetOwningActorFromActorInfo()->GetInstigatorController();
    return Cast<ARPGPartyAIController>(Controller) != nullptr;
}

ARPGPartyAIController* URPGGameplayAbility::GetPartyAIController() const
{
    if (!IsValidController())
    {
        return nullptr;
    }

    AController* Controller = GetOwningActorFromActorInfo()->GetInstigatorController();
    return Cast<ARPGPartyAIController>(Controller);
}

float URPGGameplayAbility::GetCooldown(float InLevel) const
{
    float Cooldown = 0.f;
    if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
    {
        CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
    }
    return Cooldown;
}

float URPGGameplayAbility::GetAbilityCost(float InLevel) const
{
    float TotalCost = 0.f;
    if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
    {
        for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
        {
            float ModifierValue = 0.f;
            Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ModifierValue);
            TotalCost += ModifierValue;
        }
    }
    return TotalCost;
}

void URPGGameplayAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputPressed(Handle, ActorInfo, ActivationInfo);
    OnAbilityInputPressed();
}

void URPGGameplayAbility::InputHeld(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    OnAbilityInputHeld();
}

void URPGGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);
    OnAbilityInputReleased();
}

FGenericTeamId URPGGameplayAbility::GetOwnerTeamId() const
{
	IGenericTeamAgentInterface* OwnerTeamInterface = Cast<IGenericTeamAgentInterface>(GetAvatarActorFromActorInfo());
	if (OwnerTeamInterface)
	{
		return OwnerTeamInterface->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}

void URPGGameplayAbility::PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& PushVel)
{
	TArray<AActor*> Targets = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(TargetDataHandle);
	
	for (AActor* Target : Targets)
	{
		if (!Target)
			continue;

		// Se for um Character, usa LaunchCharacter
		if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
		{
			TargetCharacter->LaunchCharacter(PushVel, true, true);
		}
		// Caso contrário, tenta aplicar força física se tiver um componente de movimento
		else if (UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
		{
			if (RootComp->IsSimulatingPhysics())
			{
				RootComp->AddImpulse(PushVel, NAME_None, true);
			}
		}
	}
} 