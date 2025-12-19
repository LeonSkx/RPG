#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_Accuracy.generated.h"

/**
 * Calcula a precisão (Accuracy) de forma híbrida:
 * - Ataques físicos: baseado em Strength
 * - Ataques mágicos: baseado em Intelligence
 */
UCLASS()
class RPG_API UMMC_Accuracy : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()

public:
    UMMC_Accuracy();

    // Implementa a lógica de cálculo de magnitude
    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
    // Captura o atributo Strength do alvo (para ataques físicos)
    FGameplayEffectAttributeCaptureDefinition StrengthDef;
    
    // Captura o atributo Intelligence do alvo (para ataques mágicos)
    FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
};

