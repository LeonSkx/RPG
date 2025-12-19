// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/Base/RPGDamageGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"
#include "RPGAbilityTypes.h"

void URPGDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	// Verificação de segurança - evitar crash se DamageEffectClass não estiver configurado
	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[RPGDamageGameplayAbility] DamageEffectClass não está configurado para %s - não aplicando dano"), *GetName());
		return;
	}

	// Verificação de segurança - evitar crash se TargetActor não existir
	if (!IsValid(TargetActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RPGDamageGameplayAbility] TargetActor é inválido - não aplicando dano"));
		return;
	}

	// Verificação de segurança - evitar crash se AbilitySystemComponent não existir
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	if (!SourceASC || !TargetASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RPGDamageGameplayAbility] SourceASC ou TargetASC é nullptr - não aplicando dano"));
		return;
	}

	// Usar o nível da habilidade ao criar o spec do efeito
	const int32 AbilityLevel = GetAbilityLevel();
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, AbilityLevel);
	
	// Verificação de segurança - evitar crash se SpecHandle não for válido
	if (!DamageSpecHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[RPGDamageGameplayAbility] DamageSpecHandle não é válido - não aplicando dano"));
		return;
	}

	const float ScaledDamage = Damage.GetValueAtLevel(AbilityLevel);
	
	// Verificação de segurança - evitar crash se DamageType não estiver configurado
	if (DamageType.IsValid())
	{
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, DamageType, ScaledDamage);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[RPGDamageGameplayAbility] DamageType não está configurado - usando dano padrão"));
	}

	// Aplicar o dano com verificação final
	if (DamageSpecHandle.Data.IsValid())
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
		UE_LOG(LogTemp, Log, TEXT("[RPGDamageGameplayAbility] Dano aplicado: %.1f do tipo %s"), ScaledDamage, *DamageType.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[RPGDamageGameplayAbility] DamageSpecHandle.Data não é válido - não aplicando dano"));
	}
}

FTaggedMontage URPGDamageGameplayAbility::GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages) const
{
	if (TaggedMontages.Num() > 0)
	{
		const int32 Selection = FMath::RandRange(0, TaggedMontages.Num() - 1);
		return TaggedMontages[Selection];
	}
	return FTaggedMontage();
}

FDamageEffectParams URPGDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor,
	FVector InRadialDamageOrigin, bool bOverrideKnockbackDirection, FVector KnockbackDirectionOverride,
	bool bOverrideDeathImpulse, FVector DeathImpulseDirectionOverride, bool bOverridePitch, float PitchOverride) const
{
	FDamageEffectParams Params;
	
	// Verificação de segurança - evitar crash se DamageEffectClass não estiver configurado
	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[RPGDamageGameplayAbility] DamageEffectClass não está configurado para %s - retornando parâmetros vazios"), *GetName());
		return Params;
	}

	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	
	// Verificação de segurança - evitar crash se SourceASC não existir
	if (!Params.SourceAbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[RPGDamageGameplayAbility] SourceAbilitySystemComponent é nullptr - retornando parâmetros vazios"));
		return Params;
	}

	Params.Attack = Damage.GetValueAtLevel(GetAbilityLevel());
	Params.AbilityLevel = GetAbilityLevel();
	Params.DamageType = DamageType;
	Params.DebuffChance = DebuffChance;
	Params.DebuffDamage = DebuffDamage;
	Params.DebuffDuration = DebuffDuration;
	Params.DebuffFrequency = DebuffFrequency;
	Params.DeathImpulseMagnitude = DeathImpulseMagnitude;
	Params.KnockbackForceMagnitude = KnockbackForceMagnitude;
	Params.KnockbackChance = KnockbackChance;

	if (IsValid(TargetActor))
	{
		FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
		if (bOverridePitch)
		{
			Rotation.Pitch = PitchOverride;
		}
		const FVector ToTarget = Rotation.Vector();
		if (!bOverrideKnockbackDirection)
		{
			Params.KnockbackForce = ToTarget * KnockbackForceMagnitude;
		}
		if (!bOverrideDeathImpulse)
		{
			Params.DeathImpulse = ToTarget * DeathImpulseMagnitude;
		}
	}
	
	if (bOverrideKnockbackDirection)
	{
		KnockbackDirectionOverride.Normalize();
		Params.KnockbackForce = KnockbackDirectionOverride * KnockbackForceMagnitude;
		if (bOverridePitch)
		{
			FRotator KnockbackRotation = KnockbackDirectionOverride.Rotation();
			KnockbackRotation.Pitch = PitchOverride;
			Params.KnockbackForce = KnockbackRotation.Vector() * KnockbackForceMagnitude;
		}
	}

	if (bOverrideDeathImpulse)
	{
		DeathImpulseDirectionOverride.Normalize();
		Params.DeathImpulse = DeathImpulseDirectionOverride * DeathImpulseMagnitude;
		if (bOverridePitch)
		{
			FRotator DeathImpulseRotation = DeathImpulseDirectionOverride.Rotation();
			DeathImpulseRotation.Pitch = PitchOverride;
			Params.DeathImpulse = DeathImpulseRotation.Vector() * DeathImpulseMagnitude;
		}
	}
	
	if (bIsRadialDamage)
	{
		Params.bIsRadialDamage = bIsRadialDamage;
		Params.RadialDamageOrigin = InRadialDamageOrigin;
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
	}
	return Params;
}

float URPGDamageGameplayAbility::GetDamageAtLevel() const
{
	// Verificação de segurança - evitar crash se Damage não estiver configurado
	if (Damage.GetValueAtLevel(1) < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RPGDamageGameplayAbility] Damage não está configurado corretamente para %s - retornando 0"), *GetName());
		return 0.f;
	}

	const float DamageValue = Damage.GetValueAtLevel(GetAbilityLevel());
	UE_LOG(LogTemp, Log, TEXT("[RPGDamageGameplayAbility] Dano no nível %d: %.1f"), GetAbilityLevel(), DamageValue);
	return DamageValue;
} 