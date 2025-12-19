#include "AbilitySystem/Calculations/MMC_Accuracy.h"

#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_Accuracy::UMMC_Accuracy()
{
    // Captura Strength para ataques físicos
    StrengthDef.AttributeToCapture = URPGAttributeSet::GetStrengthAttribute();
    StrengthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    StrengthDef.bSnapshot = false;

    // Captura Intelligence para ataques mágicos
    IntelligenceDef.AttributeToCapture = URPGAttributeSet::GetIntelligenceAttribute();
    IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    IntelligenceDef.bSnapshot = false;

    RelevantAttributesToCapture.Add(StrengthDef);
    RelevantAttributesToCapture.Add(IntelligenceDef);
}

float UMMC_Accuracy::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
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

    // Obter nível do personagem
    int32 CharacterLevel = 1;
    if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
    {
        CharacterLevel = ICombatInterface::Execute_GetCharacterLevel(Spec.GetContext().GetSourceObject());
    }

    // Base de precisão
    float BaseAccuracy = 75.0f; // 75% base
    
    // Sistema híbrido: usar o maior entre Strength e Intelligence
    // Isso permite que diferentes builds tenham boa precisão
    float PhysicalAccuracy = Strength * 0.5f; // +0.5% por ponto de Strength
    float MagicalAccuracy = Intelligence * 0.5f; // +0.5% por ponto de Intelligence
    
    // Usar o maior dos dois (sistema híbrido)
    float AttributeBonus = FMath::Max(PhysicalAccuracy, MagicalAccuracy);
    
    float Accuracy = BaseAccuracy + AttributeBonus;

    // Bônus por nível (mais experiência = mais precisão)
    Accuracy += CharacterLevel * 0.25f; // +0.25% por nível

    // Limitar entre 50% e 95%
    Accuracy = FMath::Clamp(Accuracy, 50.0f, 95.0f);

    return Accuracy;
}
