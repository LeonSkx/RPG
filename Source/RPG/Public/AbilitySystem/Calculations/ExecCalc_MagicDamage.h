#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_MagicDamage.generated.h"

/**
 * Custom Execution Calculation para dano mágico
 * Calcula dano mágico usando MagicDamage e MagicResistance
 */
UCLASS()
class RPG_API UExecCalc_MagicDamage : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    UExecCalc_MagicDamage();

    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
