#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxHealth.generated.h"

/**
 * Calcula a vida máxima com base no atributo Vigor e no nível do jogador
 */
UCLASS()
class RPG_API UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()

public:
    UMMC_MaxHealth();

    // Implementa a lógica de cálculo de magnitude
    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
    // Captura o atributo Vigor do alvo
    FGameplayEffectAttributeCaptureDefinition VigorDef;
}; 