#include "Actor/Potion/ARPGHealEffectActor.h"
#include "Character/RPGCharacter.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"

ARPGHealEffectActor::ARPGHealEffectActor()
{
    // Herda configuração de tick e root do ARPGEffectActor
}

void ARPGHealEffectActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (ARPGCharacter* Character = Cast<ARPGCharacter>(OtherActor))
    {
        UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
        if (ASC)
        {
            const float Current = ASC->GetNumericAttribute(URPGAttributeSet::GetHealthAttribute());
            const float Max     = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxHealthAttribute());
            if (Current < Max)
            {
                // Aplica cura configurada via GameplayEffect
                ApplyEffectToTarget(Character, InstantGameplayEffectClass);
                Destroy();
            }
        }
    }
}

// Override de ApplyEffectToTarget para incluir verificação de health
void ARPGHealEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
    if (ARPGCharacter* Character = Cast<ARPGCharacter>(TargetActor))
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
        {
            const float Current = ASC->GetNumericAttribute(URPGAttributeSet::GetHealthAttribute());
            const float Max     = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxHealthAttribute());
            if (Current < Max)
            {
                Super::ApplyEffectToTarget(TargetActor, GameplayEffectClass);
                Destroy();
            }
        }
    }
} 