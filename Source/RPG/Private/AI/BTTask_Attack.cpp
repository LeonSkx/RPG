// Copyright Druid Mechanics


#include "AI/BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "AbilitySystemInterface.h"


UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	APawn* AIPawn = AIController->GetPawn();
	if (AIPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(GetSelectedBlackboardKey()));

	if (TargetActor == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// Log removed for cleanup
	
	const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(AIPawn);
	if (ASI)
	{
		UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
		if (ASC)
		{
			FGameplayTagContainer AttackTags;
			AttackTags.AddTag(FRPGGameplayTags::Get().Abilities_Attack);
			ASC->TryActivateAbilitiesByTag(AttackTags);
		}
	}

	return EBTNodeResult::Succeeded;
}
