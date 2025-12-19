#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"
#include "AbilitySystem/Abilities/Magic/Lazer/TargetActor_Line.h"
#include "GA_Lazer.generated.h"

/**
 * Channeled lazer ability that applies damage to targets in front of the caster while draining mana.
 */
UCLASS()
class RPG_API UGA_Lazer : public URPGGameplayAbility
{
    GENERATED_BODY()

public:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

    static FGameplayTag GetShootTag();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Targeting")
    float TargetRange = 4000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Targeting")
    float DetectionCylinderRadius = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Targeting")
    float TargetingInterval = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> HitDamageEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    float HitPushSpeed = 3000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> OnGoingConsumtionEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Anim")
    class UAnimMontage* LazerMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Targeting")
    FName TargetActorAttachSocketName = "Lazer";

    UPROPERTY(EditDefaultsOnly, Category = "Targeting")
    TSubclassOf<class ATargetActor_Line> LazerTargetActorClass;

    FActiveGameplayEffectHandle OnGoingConsumtionEffectHandle;
    FDelegateHandle ManaChangeDelegateHandle;

    UFUNCTION()
    void ShootLazer(FGameplayEventData Payload);

    void ManaUpdated(const FOnAttributeChangeData& ChangeData);

    UFUNCTION()
    void TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
};

