#include "AbilitySystem/Calculations/MMC_MagicDamage.h"

#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MagicDamage::UMMC_MagicDamage()
{
    // Captura Intelligence para ataques mágicos principais (Source - quem ataca)
    IntelligenceDef.AttributeToCapture = URPGAttributeSet::GetIntelligenceAttribute();
    IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
    IntelligenceDef.bSnapshot = false;

    // Captura Strength para ataques híbridos (Source - quem ataca)
    StrengthDef.AttributeToCapture = URPGAttributeSet::GetStrengthAttribute();
    StrengthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
    StrengthDef.bSnapshot = false;

    RelevantAttributesToCapture.Add(IntelligenceDef);
    RelevantAttributesToCapture.Add(StrengthDef);
}

float UMMC_MagicDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // Gather tags from source and target
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;

    // Capturar Intelligence e Strength
    float Intelligence = 0.f;
    GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvaluationParameters, Intelligence);
    Intelligence = FMath::Max<float>(Intelligence, 0.f);

    float Strength = 0.f;
    GetCapturedAttributeMagnitude(StrengthDef, Spec, EvaluationParameters, Strength);
    Strength = FMath::Max<float>(Strength, 0.f);

    // Obter nível do personagem (Source - quem dispara o ataque)
    int32 CharacterLevel = 1;
    if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
    {
        CharacterLevel = ICombatInterface::Execute_GetCharacterLevel(Spec.GetContext().GetSourceObject());
    }

    // Base de MagicDamage
    float BaseMagicDamage = 8.0f; // 8 pontos base (menor que Attack físico)
    
    // Bônus por atributos
    float IntelligenceBonus = Intelligence * 2.5f; // +2.5 pontos por ponto de Intelligence
    float StrengthBonus = Strength * 0.5f; // +0.5 pontos por ponto de Strength (híbrido)
    
    // Bônus por nível (progressão mágica)
    float LevelBonus = CharacterLevel * 4.0f; // +4 pontos por nível (mais que Attack)
    
    // Cálculo final
    float MagicDamage = BaseMagicDamage + IntelligenceBonus + StrengthBonus + LevelBonus;
    
    // Garantir valor mínimo
    MagicDamage = FMath::Max(MagicDamage, 1.0f);
    
    return MagicDamage;
}
