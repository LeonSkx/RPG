// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Base/RPGDamageGameplayAbility.h"
#include "RPGFireBlast.generated.h"

class ARPGFireBall;

/**
 * Habilidade que dispara múltiplas bolas de fogo em todas as direções
 * As bolas saem, fazem uma trajetória e retornam para explodir
 */
UCLASS()
class RPG_API URPGFireBlast : public URPGDamageGameplayAbility
{
	GENERATED_BODY()
public:
	FString GetDescription(int32 Level);
	FString GetNextLevelDescription(int32 Level);

	UFUNCTION(BlueprintCallable)
	TArray<ARPGFireBall*> SpawnFireBalls();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "FireBlast")
	int32 NumFireBalls = 12;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ARPGFireBall> FireBallClass;
}; 