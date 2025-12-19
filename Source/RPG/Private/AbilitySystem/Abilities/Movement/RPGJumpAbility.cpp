// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/Movement/RPGJumpAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

URPGJumpAbility::URPGJumpAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void URPGJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
									  const FGameplayAbilityActorInfo* ActorInfo,
									  const FGameplayAbilityActivationInfo ActivationInfo,
									  const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// Não executar pulo automaticamente - aguardar chamada do Blueprint
}

bool URPGJumpAbility::TryExecuteJump()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		return false;
	}

	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return false;
	}

	// Verificar se pode pular (não está caindo)
	if (MovementComp->IsFalling())
	{
		return false;
	}

	// Aplicar configurações personalizadas se habilitado
	if (bUseCustomJumpSettings)
	{
		MovementComp->JumpZVelocity = JumpZVelocity;
		MovementComp->AirControl = AirControl;
	}

	// Executar o pulo usando a nova API
	MovementComp->DoJump(false, GetWorld()->GetDeltaSeconds());
	
	return true;
}

bool URPGJumpAbility::ExecuteJump()
{
	return TryExecuteJump();
}

