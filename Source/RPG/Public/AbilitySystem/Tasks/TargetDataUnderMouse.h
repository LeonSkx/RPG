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
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);

	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature ValidData;

    // Se true (default), replica TargetData; se false, opera localmente sem replicar
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability|Tasks")
    bool bReplicateTargetData = false;

	// Distância máxima do trace (em cm). Se <= 0, usa MaxTargetingRange do personagem
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability|Tasks", meta = (ClampMin = "0.0"))
	float MaxTraceRange = 3000.f;

	// Se true, desenha debug do trace (linha e ponto de impacto)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability|Tasks|Debug")
	bool bDrawDebugTrace = true;

private:

	virtual void Activate() override;
	void SendMouseCursorData();

	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
