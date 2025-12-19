// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISense_Sight.h"
#include "Engine/Engine.h"
#include "Interaction/CombatInterface.h"
#include "RPGPartyAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
struct FAIStimulus;

class ARPGCharacterBase;

/**
 * 
 */
UCLASS()
class RPG_API ARPGPartyAIController : public AAIController
{
	GENERATED_BODY()
public:
	ARPGPartyAIController();

	/** Atualiza a localização conhecida do alvo atual */
	UFUNCTION(BlueprintCallable, Category = "AI")
	void UpdateTargetLocation(const FVector& NewLocation);

	/** Atualiza PartyMember apenas quando está em paz (sem inimigos, sons ou dano) */
	UFUNCTION()
	void UpdatePartyMemberWhenSafe();

  /** Força o controlador a alinhar seu estado com o PartySubsystem (ativo x não-ativo) */
  UFUNCTION(BlueprintCallable, Category = "AI")
  void RefreshActiveState();

protected:
	virtual void OnPossess(APawn* InPawn) override;
  virtual void OnUnPossess() override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	void HandleTargetDeath(AActor* DeadActor);

  UFUNCTION()
  void HandleActiveMemberChanged(class ARPGCharacter* NewActive);

  UFUNCTION()
  void HandleOwnerDeath(AActor* DeadActor);

	void SetTarget(ARPGCharacterBase* NewTarget);
	void ClearTarget();
	
	UPROPERTY()
	TObjectPtr<ARPGCharacterBase> TargetCharacter;

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

private:
	/** Timer handle para atualizar PartyMember quando seguro */
	FTimerHandle UpdatePartyMemberTimerHandle;

  /** Marca se já vinculamos ao PartySubsystem para eventos */
  bool bBoundToPartySubsystem = false;
}; 