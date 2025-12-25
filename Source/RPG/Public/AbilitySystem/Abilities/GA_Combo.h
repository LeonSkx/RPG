// Copyright (c) 2025 RPG Yumi Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"
#include "GA_Combo.generated.h"

/**
 * Ability de combo que permite encadear ataques baseado em input do jogador
 */
UCLASS()
class RPG_API UGA_Combo : public URPGGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Combo();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	static FGameplayTag GetComboChangedEventTag();
	static FGameplayTag GetComboChangedEventEndTag();

private:
	void SetupWaitComboInputPress();

	UFUNCTION()
	void HandleInputPress(FGameplayEventData EventData);

	void TryCommitCombo();

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ComboMontage;

	UFUNCTION()
	void ComboChangedEventReceived(FGameplayEventData Data);

	FName NextComboName;

	/** Referência para a task que escuta input pressionado */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitInputPressedTask;
};