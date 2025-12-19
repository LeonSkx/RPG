// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "RPGProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class RPG_API ARPGProjectile : public AActor
{
    GENERATED_BODY()

public:
    ARPGProjectile();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
    UPROPERTY(BlueprintReadWrite, meta=(ExposeOnSpawn=true))
    FGameplayEffectSpecHandle DamageEffectSpecHandle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> Sphere;

protected:
    virtual void BeginPlay() override;
    virtual void Destroyed() override;

    UFUNCTION()
    void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                         bool bFromSweep, const FHitResult& SweepResult);

    // Função virtual para ser sobrescrita pelas classes filhas
    UFUNCTION(BlueprintCallable)
    virtual void OnHit();

private:
    // Tempo de vida antes de autodestruição
    UPROPERTY(EditDefaultsOnly)
    float LifeSpan = 15.f;

    // Indica que houve impacto sem autoridade, para suprimir som de Destroyed
    bool bHit = false;


}; 