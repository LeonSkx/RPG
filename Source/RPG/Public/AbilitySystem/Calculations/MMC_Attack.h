#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_Attack.generated.h"

/**
 * Modifier Magnitude Calculation para o atributo Attack
 * 
 * Calcula o Attack baseado em:
 * - Strength (ataques físicos)
 * - Intelligence (ataques mágicos/híbridos)  
 * - Nível do personagem (progressão)
 */
UCLASS()
class RPG_API UMMC_Attack : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_Attack();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition StrengthDef;
	FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
};
