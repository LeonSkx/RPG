// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignature, const FGameplayAbilityTargetDataHandle&, DataHandle);
/**
 * 
 */
UCLASS()
class RPG_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (DisplayName = "TargetDataUnderMouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(
		UGameplayAbility* OwningAbility,
		float MaxTraceRange = 3000.f,
		bool bDrawDebugTrace = false,
		bool bUsePawnViewPoint = false
	);

	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature ValidData;

    // Se true (default), replica TargetData; se false, opera localmente sem replicar
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability|Tasks")
    bool bReplicateTargetData = false;

private:

	// Distância máxima do trace (em cm). Se <= 0, usa MaxTargetingRange do personagem
	float MaxTraceRange = 3000.f;

	// Se true, desenha debug do trace (linha e ponto de impacto)
	bool bDrawDebugTrace = false;

	// Se true, usa a visão do Pawn (ActorLocation + ControlRotation); se false, usa PlayerController ViewPoint
	bool bUsePawnViewPoint = false;

	virtual void Activate() override;
	void SendMouseCursorData();

	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
