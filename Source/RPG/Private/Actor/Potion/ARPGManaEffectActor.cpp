#include "Actor/Potion/ARPGManaEffectActor.h"
#include "Character/RPGCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"

ARPGManaEffectActor::ARPGManaEffectActor()
{
    // Herdando configuração de Tick e root de ARPGEffectActor
}

void ARPGManaEffectActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (ARPGCharacter* Character = Cast<ARPGCharacter>(OtherActor))
    {
        UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
        if (ASC)
        {
            const float Current = ASC->GetNumericAttribute(URPGAttributeSet::GetManaAttribute());
            const float Max     = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxManaAttribute());
            if (Current < Max)
            {
                // Aplica efeito de mana configurado em InstantGameplayEffectClass
                ApplyEffectToTarget(Character, InstantGameplayEffectClass);
                Destroy();
            }
        }
    }
}

void ARPGManaEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
    if (ARPGCharacter* Character = Cast<ARPGCharacter>(TargetActor))
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
        {
            const float Current = ASC->GetNumericAttribute(URPGAttributeSet::GetManaAttribute());
            const float Max     = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxManaAttribute());
            if (Current < Max)
            {
                Super::ApplyEffectToTarget(TargetActor, GameplayEffectClass);
                Destroy();
            }
        }
    }
} 