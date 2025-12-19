// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PartyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPartyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface para membros da party
 */
class RPG_API IPartyInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetCombatTarget(AActor* InCombatTarget);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetCombatTarget() const;
}; 