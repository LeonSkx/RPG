// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Base/RPGDamageGameplayAbility.h"
#include "RPGProjectileSpell.generated.h"

class ARPGProjectile;
class UGameplayEffect;

/**
 * Habilidade de projétil que instancia um projétil e dispara sobre input
 */
UCLASS()
class RPG_API URPGProjectileSpell : public URPGDamageGameplayAbility
{
    GENERATED_BODY()

protected:
    /** Executa a habilidade de projétil */
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                 const FGameplayAbilityActorInfo* ActorInfo,
                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                 const FGameplayEventData* TriggerEventData) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void SpawnProjectile(const FVector& ProjectileTargetLocation);

    /** Classe do projétil a ser instanciado */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ability")
    TSubclassOf<ARPGProjectile> ProjectileClass;
}; 