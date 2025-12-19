// Copyright Druid Mechanics


#include "AI/BTService_UpdateDistance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/Engine.h"

UBTService_UpdateDistance::UBTService_UpdateDistance()
{
	NodeName = "Update Distance To Target";
}

void UBTService_UpdateDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	DebugTimer += DeltaSeconds;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (Blackboard == nullptr)
	{
		return;
	}

	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorSelector.SelectedKeyName));
	AAIController* AIController = OwnerComp.GetAIOwner();
	
	if (TargetActor && AIController && AIController->GetPawn())
	{
		// Alvo direto disponível
		const float Distance = AIController->GetPawn()->GetDistanceTo(TargetActor);
		Blackboard->SetValueAsFloat(DistanceToTargetSelector.SelectedKeyName, Distance);
		
		if (GEngine && DebugTimer > 1.f)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				2.f, // Display for 2 seconds
				FColor::Yellow, 
				FString::Printf(TEXT("Distance to %s: %f"), *TargetActor->GetName(), Distance)
			);
			DebugTimer = 0.f;
		}
	}
	else
	{
		// Verificar se há uma localização conhecida do alvo
		FVector LastKnownLocation = Blackboard->GetValueAsVector(TEXT("LastKnownLocation"));
		if (LastKnownLocation != FVector::ZeroVector && AIController && AIController->GetPawn())
		{
			const float Distance = FVector::Dist(AIController->GetPawn()->GetActorLocation(), LastKnownLocation);
			Blackboard->SetValueAsFloat(DistanceToTargetSelector.SelectedKeyName, Distance);
			
			if (GEngine && DebugTimer > 1.f)
			{
				GEngine->AddOnScreenDebugMessage(
					1,
					2.f, // Display for 2 seconds
					FColor::Orange, 
					FString::Printf(TEXT("Distance to LastKnownLocation: %f"), Distance)
				);
				DebugTimer = 0.f;
			}
		}
		else
		{
			Blackboard->ClearValue(DistanceToTargetSelector.SelectedKeyName);
		}
	}
} 