// Copyright Druid Mechanics


#include "AbilitySystem/Core/RPGAbilitySystemGlobals.h"
#include "RPGAbilityTypes.h"

FGameplayEffectContext* URPGAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FRPGGameplayEffectContext();
} 