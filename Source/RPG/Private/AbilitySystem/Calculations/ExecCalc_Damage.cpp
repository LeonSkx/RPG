#include "AbilitySystem/Calculations/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Core/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "AbilitySystem/Data/EnemyClassInfo.h"
#include "Interaction/CombatInterface.h"

struct RPGDamageStatics
{

	
	// Defensive attributes (Target)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	
	// Offensive attributes (Source)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Attack);
	
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	
	RPGDamageStatics()
	{
		
		
		// Defensive attributes (Target)
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, Armor, Target, false);
		
		// Offensive attributes (Source)
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, Attack, Source, false);

		const FRPGGameplayTags& Tags = FRPGGameplayTags::Get();
		
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Attack, AttackDef);
	}
};

static const RPGDamageStatics& DamageStatics()
{
	static RPGDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{

	
	// Defensive attributes (Target)
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	
	// Offensive attributes (Source)
	RelevantAttributesToCapture.Add(DamageStatics().AttackDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	// Verificação de segurança - evitar crash se componentes não existirem
	if (!SourceASC || !TargetASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ExecCalc_Damage] SourceASC ou TargetASC é nullptr - cancelando execução"));
		return;
	}

	AActor* SourceAvatar = SourceASC->GetAvatarActor();
	AActor* TargetAvatar = TargetASC->GetAvatarActor();
	
	// Verificação de segurança - evitar crash se avatares não existirem
	if (!SourceAvatar || !TargetAvatar)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ExecCalc_Damage] SourceAvatar ou TargetAvatar é nullptr - cancelando execução"));
		return;
	}

	ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvatar);

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	// Verificação de segurança - evitar crash se tags não existirem
	if (!SourceTags || !TargetTags)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ExecCalc_Damage] SourceTags ou TargetTags é nullptr - cancelando execução"));
		return;
	}
	
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
        Damage = Spec.GetSetByCallerMagnitude(Tags.Damage_Physical, false, 0.f);
    }
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
    
    // Capturar Attack do Source (personagem + equipamentos)
    float SourceAttack = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackDef, EvaluationParameters, SourceAttack);
    SourceAttack = FMath::Max(0.f, SourceAttack);



	// Sistema de Block removido - pode ser implementado como habilidade especial
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	// Sistema de resistências baseado no tipo de dano
	
	// Verificar tipo de dano e aplicar resistência correta
	float ResistanceReduction = 0.f;
	
	if (SourceTags->HasTag(Tags.Damage_Physical))
	{
		// Dano físico - usar Armor
		float TargetArmor = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
		TargetArmor = FMath::Max<float>(TargetArmor, 0.f);
		ResistanceReduction = TargetArmor;
	}
	else if (SourceTags->HasTag(Tags.Damage_Fire))
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
	
	// Aplicar redução de resistência (com verificação de segurança)
	if (ResistanceReduction >= 100.f)
	{
		// Resistência total - dano reduzido a 0
		Damage = 0.f;
		UE_LOG(LogTemp, Log, TEXT("[ExecCalc_Damage] Resistência total aplicada - dano reduzido a 0"));
	}
	else if (ResistanceReduction > 0.f)
	{
		// Aplicar redução de resistência
		Damage *= (100.f - ResistanceReduction) / 100.f;
		UE_LOG(LogTemp, Log, TEXT("[ExecCalc_Damage] Resistência aplicada: %.1f%%, Dano final: %.1f"), ResistanceReduction, Damage);
	}

	// Sistema de Critical Hit removido - pode ser implementado como habilidade especial
	
    // Aplicar Attack do personagem ao dano base da habilidade
    Damage = Damage + SourceAttack;
    
    // Verificação final de segurança - garantir que o dano não seja negativo
    Damage = FMath::Max(0.f, Damage);
    
    // Log do dano final para debug
    UE_LOG(LogTemp, Log, TEXT("[ExecCalc_Damage] Dano final calculado: %.1f (Base: %.1f, Attack: %.1f)"), Damage, Damage - SourceAttack, SourceAttack);

	// Só aplicar dano se for maior que 0
	if (Damage > 0.f)
	{
		const FGameplayModifierEvaluatedData EvaluatedData(URPGAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
		OutExecutionOutput.AddOutputModifier(EvaluatedData);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ExecCalc_Damage] Dano é 0 ou negativo - não aplicando dano"));
	}
}
