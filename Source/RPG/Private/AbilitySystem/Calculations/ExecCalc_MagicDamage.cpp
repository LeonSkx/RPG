                                                 #include "AbilitySystem/Calculations/ExecCalc_MagicDamage.h"

#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Core/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "AbilitySystem/Data/EnemyClassInfo.h"
#include "Interaction/CombatInterface.h"

struct RPGMagicDamageStatics
{

	
	// Defensive attributes (Target)
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicResistance);
	
	// Offensive attributes (Source)
	DECLARE_ATTRIBUTE_CAPTUREDEF(MagicDamage);
	
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	
	RPGMagicDamageStatics()
	{

		
		// Defensive attributes (Target)
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, MagicResistance, Target, false);
		
		// Offensive attributes (Source)
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, MagicDamage, Source, false);

		const FRPGGameplayTags& Tags = FRPGGameplayTags::Get();
		
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_MagicResistance, MagicResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_MagicDamage, MagicDamageDef);
	}
};

static const RPGMagicDamageStatics& MagicDamageStatics()
{
	static RPGMagicDamageStatics MDStatics;
	return MDStatics;
}

UExecCalc_MagicDamage::UExecCalc_MagicDamage()
{

	
	// Defensive attributes (Target)
	RelevantAttributesToCapture.Add(MagicDamageStatics().MagicResistanceDef);
	
	// Offensive attributes (Source)
	RelevantAttributesToCapture.Add(MagicDamageStatics().MagicDamageDef);
}

void UExecCalc_MagicDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;
	ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvatar);

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

    // Get Damage Set by Caller Magnitude (habilidade base)
    float Damage = 0.f;
    // Tentar capturar o dano usando as tags de tipo de dano
    const FRPGGameplayTags& Tags = FRPGGameplayTags::Get();
    Damage = Spec.GetSetByCallerMagnitude(Tags.Damage, false, 0.f);
    if (Damage == 0.f)
    {
        Damage = Spec.GetSetByCallerMagnitude(Tags.Damage_Fire, false, 0.f);
    }
    if (Damage == 0.f)
    {
        Damage = Spec.GetSetByCallerMagnitude(Tags.Damage_Lightning, false, 0.f);
    }
    if (Damage == 0.f)
    {
        Damage = Spec.GetSetByCallerMagnitude(Tags.Damage_Arcane, false, 0.f);
    }
    Damage = FMath::Max(0.f, Damage);
    
    // Capturar MagicDamage do Source (personagem + equipamentos)
    float SourceMagicDamage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicDamageStatics().MagicDamageDef, EvaluationParameters, SourceMagicDamage);
    SourceMagicDamage = FMath::Max(0.f, SourceMagicDamage);

	// Sistema de Block removido - pode ser implementado como habilidade especial
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	// Sistema de resistências baseado no tipo de dano mágico
	
	// Verificar tipo de dano mágico e aplicar resistência correta
	float ResistanceReduction = 0.f;
	
	if (SourceTags->HasTag(Tags.Damage_Fire))
	{
		// Dano de fogo - usar Fire Resistance
		float FireResistance = 0.f;
		// TODO: Capturar FireResistance do Target
		ResistanceReduction = FireResistance;
	}
	else if (SourceTags->HasTag(Tags.Damage_Lightning))
	{
		// Dano de relâmpago - usar Lightning Resistance
		float LightningResistance = 0.f;
		// TODO: Capturar LightningResistance do Target
		ResistanceReduction = LightningResistance;
	}
	else if (SourceTags->HasTag(Tags.Damage_Arcane))
	{
		// Dano arcano - usar Arcane Resistance
		float ArcaneResistance = 0.f;
		// TODO: Capturar ArcaneResistance do Target
		ResistanceReduction = ArcaneResistance;
	}
	else
	{
		// Dano mágico genérico - usar MagicResistance
		float TargetMagicResistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicDamageStatics().MagicResistanceDef, EvaluationParameters, TargetMagicResistance);
		TargetMagicResistance = FMath::Max<float>(TargetMagicResistance, 0.f);
		ResistanceReduction = TargetMagicResistance;
	}
	
	// Aplicar redução de resistência
	Damage *= (100.f - ResistanceReduction) / 100.f;

	// Sistema de Critical Hit removido - pode ser implementado como habilidade especial
	
    // Aplicar MagicDamage do personagem ao dano base da habilidade
    Damage = Damage + SourceMagicDamage;

	const FGameplayModifierEvaluatedData EvaluatedData(URPGAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
