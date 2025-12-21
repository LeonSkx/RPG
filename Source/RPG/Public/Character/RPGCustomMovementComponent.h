// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RPGCustomMovementComponent.generated.h"

/**
 * Character Movement Component customizado para o RPG
 */
UCLASS()
class RPG_API URPGCustomMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	URPGCustomMovementComponent(const FObjectInitializer& ObjectInitializer);
};

