// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"
#include "RPGJumpAbility.generated.h"

UCLASS()
class RPG_API URPGJumpAbility : public URPGGameplayAbility
{
	GENERATED_BODY()

public:
	URPGJumpAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
								const FGameplayAbilityActorInfo* ActorInfo,
								const FGameplayAbilityActivationInfo ActivationInfo,
								const FGameplayEventData* TriggerEventData) override;

	// Se deve usar configurações personalizadas de pulo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump Settings")
	bool bUseCustomJumpSettings = false;

	// Força do pulo (altura)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump Settings", 
		meta = (EditCondition = "bUseCustomJumpSettings"))
	float JumpZVelocity = 600.0f;

	// Controle no ar
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Jump Settings", 
		meta = (EditCondition = "bUseCustomJumpSettings"))
	float AirControl = 0.2f;

private:
	bool TryExecuteJump();

public:
	// Função para Blueprint executar o pulo após animações
	UFUNCTION(BlueprintCallable, Category = "Jump")
	bool ExecuteJump();
};