#include "AbilitySystem/Calculations/MMC_Attack.h"

#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_Attack::UMMC_Attack()
{
    // Captura Strength para ataques físicos (Source - quem ataca)
    StrengthDef.AttributeToCapture = URPGAttributeSet::GetStrengthAttribute();
    StrengthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
    StrengthDef.bSnapshot = false;

    // Captura Intelligence para ataques mágicos/híbridos (Source - quem ataca)
    IntelligenceDef.AttributeToCapture = URPGAttributeSet::GetIntelligenceAttribute();
    IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
    IntelligenceDef.bSnapshot = false;

    RelevantAttributesToCapture.Add(StrengthDef);
    RelevantAttributesToCapture.Add(IntelligenceDef);
}

float UMMC_Attack::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // Gather tags from source and target
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;

    // Capturar Strength e Intelligence
    float Strength = 0.f;
    GetCapturedAttributeMagnitude(StrengthDef, Spec, EvaluationParameters, Strength);
    Strength = FMath::Max<float>(Strength, 0.f);

    float Intelligence = 0.f;
    GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvaluationParameters, Intelligence);
    Intelligence = FMath::Max<float>(Intelligence, 0.f);

    // Obter nível do personagem (Source - quem dispara o ataque)
    int32 CharacterLevel = 1;
    if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
    {
        CharacterLevel = ICombatInterface::Execute_GetCharacterLevel(Spec.GetContext().GetSourceObject());
    }

    // Base de Attack
    float BaseAttack = 10.0f; // 10 pontos base
    
    // Bônus por atributos
    float StrengthBonus = Strength * 2.0f; // +2 pontos por ponto de Strength
    float IntelligenceBonus = Intelligence * 1.5f; // +1.5 pontos por ponto de Intelligence
    
    // Bônus por nível (progressão)
    float LevelBonus = CharacterLevel * 3.0f; // +3 pontos por nível
    
    // Cálculo final
    float Attack = BaseAttack + StrengthBonus + IntelligenceBonus + LevelBonus;
    
    // Garantir valor mínimo
    Attack = FMath::Max(Attack, 1.0f);
    
    return Attack;
}
