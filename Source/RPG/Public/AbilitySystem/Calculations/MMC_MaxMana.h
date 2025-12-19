#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxMana.generated.h"

/**
 * Calcula a mana máxima com base no atributo Inteligência e no nível do jogador
 */
UCLASS()
class RPG_API UMMC_MaxMana : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()

public:
    UMMC_MaxMana();

    // Implementa a lógica de cálculo de magnitude
    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
    // Captura o atributo Inteligência do alvo
    FGameplayEffectAttributeCaptureDefinition IntDef;
}; 