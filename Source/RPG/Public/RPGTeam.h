// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "RPGTeam.generated.h"

UENUM(BlueprintType)
enum class ERPGTeam : uint8
{
	Player,
	Enemy,
	Neutral
}; 