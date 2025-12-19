// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISense_Sight.h"
#include "Engine/Engine.h"
#include "Interaction/CombatInterface.h"
#include "RPGAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
struct FAIStimulus;

class ARPGCharacterBase;

/**
 * 
 */
UCLASS()
class RPG_API ARPGAIController : public AAIController
{
	GENERATED_BODY()
public:
	ARPGAIController();

	/** Atualiza a localização conhecida do alvo atual */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateTargetLocation(const FVector& NewLocation);

protected:
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void HandleTargetDeath(AActor* DeadActor);

	void SetTarget(ARPGCharacterBase* NewTarget);
	void ClearTarget();
	
	UPROPERTY()
	TObjectPtr<ARPGCharacterBase> TargetCharacter;

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;
}; 