#pragma once

#include "CoreMinimal.h"
#include "Actor/RPGEffectActor.h"
#include "ARPGManaEffectActor.generated.h"

UCLASS()
class RPG_API ARPGManaEffectActor : public ARPGEffectActor
{
    GENERATED_BODY()

public:
    ARPGManaEffectActor();

    // Override de ApplyEffectToTarget para checar Mana antes de aplicar e destruir
    virtual void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass) override;

protected:
    /** Override do begin overlap para aplicar efeito de mana e destruir baseado em Mana */
    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
}; 