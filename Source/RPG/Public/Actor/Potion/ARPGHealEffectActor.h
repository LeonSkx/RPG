#pragma once

#include "CoreMinimal.h"
#include "Actor/RPGEffectActor.h"
#include "ARPGHealEffectActor.generated.h"

UCLASS()
class RPG_API ARPGHealEffectActor : public ARPGEffectActor
{
    GENERATED_BODY()

public:
    ARPGHealEffectActor();

    // Override de ApplyEffectToTarget para checar health antes de curar
    virtual void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass) override;

protected:
    /** Override do begin overlap para aplicar cura e destruir baseado em HP */
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
}; 