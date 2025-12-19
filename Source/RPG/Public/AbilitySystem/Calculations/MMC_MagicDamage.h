#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MagicDamage.generated.h"

/**
 * Modifier Magnitude Calculation para o atributo MagicDamage
 * 
 * Calcula o MagicDamage baseado em:
 * - Intelligence (ataques mágicos principais)
 * - Strength (ataques híbridos)
 * - Nível do personagem (progressão mágica)
 */
UCLASS()
class RPG_API UMMC_MagicDamage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UMMC_MagicDamage();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
	FGameplayEffectAttributeCaptureDefinition StrengthDef;
};
