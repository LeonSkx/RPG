// Copyright Druid Mechanics

#include "AI/RPGAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/RPGCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Hearing.h"
#include "Engine/Engine.h"
#include "Interaction/CombatInterface.h"
#include "Character/RPGCharacterBase.h"
#include "GenericTeamAgentInterface.h"
#include "Character/RPGEnemy.h"


ARPGAIController::ARPGAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerceptionComponent");
	SetPerceptionComponent(*AIPerceptionComponent);

	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	SightConfig->SightRadius = 1000.f;
	SightConfig->LoseSightRadius = 1300.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->SetMaxAge(8.f);
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("Damage Config"));

	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing Config"));
	HearingConfig->HearingRange = 2000.f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->SetMaxAge(20.f);

	GetAIPerceptionComponent()->ConfigureSense(*SightConfig);
	GetAIPerceptionComponent()->ConfigureSense(*DamageConfig);
	GetAIPerceptionComponent()->ConfigureSense(*HearingConfig);

	GetAIPerceptionComponent()->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ARPGAIController::SetTarget(ARPGCharacterBase* NewTarget)
{
    // Nunca setar alvos mortos
    if (!NewTarget || (NewTarget->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(NewTarget)))
    {
        return;
    }

	if (IsValid(TargetCharacter))
	{
		TargetCharacter->GetOnDeathDelegate().RemoveAll(this);
	}

	TargetCharacter = NewTarget;

	if (IsValid(TargetCharacter))
	{
		TargetCharacter->GetOnDeathDelegate().AddDynamic(this, &ARPGAIController::HandleTargetDeath);
	}
	
	GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), NewTarget);

	// Atualizar o CombatTarget do inimigo com o novo alvo
	if (ARPGEnemy* Enemy = Cast<ARPGEnemy>(GetPawn()))
	{
		Enemy->SetCombatTarget_Implementation(NewTarget);
		
		// Se há um alvo válido, atualizar a localização conhecida
		if (IsValid(NewTarget))
		{
			GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), NewTarget->GetActorLocation());
		}
	}
}

void ARPGAIController::ClearTarget()
{
	if (IsValid(TargetCharacter))
	{
		TargetCharacter->GetOnDeathDelegate().RemoveAll(this);
	}
	TargetCharacter = nullptr;
	GetBlackboardComponent()->ClearValue(TEXT("TargetActor"));

	// Limpar o CombatTarget do inimigo
	if (ARPGEnemy* Enemy = Cast<ARPGEnemy>(GetPawn()))
	{
		Enemy->SetCombatTarget_Implementation(nullptr);
	}
}

void ARPGAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	GetAIPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ARPGAIController::OnTargetPerceptionUpdated);
}

void ARPGAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;
	auto* RPGCharacter = Cast<ARPGCharacterBase>(Actor);
	if (!RPGCharacter) return;

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		// Verificar se é do mesmo tipo de inimigo (não reagir ao dano de aliados)
		ARPGEnemy* MyEnemy = Cast<ARPGEnemy>(GetPawn());
		ARPGEnemy* OtherEnemy = Cast<ARPGEnemy>(RPGCharacter);
		
		if (MyEnemy && OtherEnemy)
		{
			// Se ambos são inimigos, ignorar o dano
			return;
		}
		
        if (!IsValid(TargetCharacter))
		{
			// Verificar distância antes de definir o alvo
			const float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), RPGCharacter->GetActorLocation());
			const float MaxDamageResponseDistance = 1000.0f; // Igual ao raio de visão para consistência
			
            // Não considerar alvos mortos
            const bool bTargetAlive = !(RPGCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(RPGCharacter));
            if (DistanceToTarget <= MaxDamageResponseDistance && bTargetAlive)
			{
				SetTarget(RPGCharacter);
			}
		}
		else
		{
			// Se já tem um alvo, atualiza a LastKnownLocation com a posição do dano
			GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), RPGCharacter->GetActorLocation());
		}
		return;
	}

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		GetBlackboardComponent()->SetValueAsVector(TEXT("NoiseLocation"), Stimulus.StimulusLocation);
		// Removido: Atualização de LastKnownLocation ao ouvir som
		return;
	}

	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		// Verificar se é do mesmo tipo de inimigo (não atacar aliados)
		ARPGEnemy* MyEnemy = Cast<ARPGEnemy>(GetPawn());
		ARPGEnemy* OtherEnemy = Cast<ARPGEnemy>(RPGCharacter);
		
		if (MyEnemy && OtherEnemy)
		{
			// Se ambos são inimigos, não atacar
			return;
		}
		
		const bool bIsHostile = GetTeamAttitudeTowards(*RPGCharacter) == ETeamAttitude::Hostile;
		
		// Log removed for cleanup
		
		if (!bIsHostile) return;
		
        if (Stimulus.WasSuccessfullySensed())
		{
            // Não considerar alvos mortos
            const bool bTargetAlive = !(RPGCharacter->Implements<UCombatInterface>() && ICombatInterface::Execute_IsDead(RPGCharacter));
            if (!IsValid(TargetCharacter) && bTargetAlive)
			{
				SetTarget(RPGCharacter);
			}
			else if (TargetCharacter == RPGCharacter)
			{
				// Atualizar localização conhecida do alvo atual
				GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), RPGCharacter->GetActorLocation());
			}
		}
		else // Alvo de visão perdido
		{
			if (TargetCharacter == RPGCharacter)
			{
				GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), TargetCharacter->GetActorLocation());
				ClearTarget();
			}
		}
	}
}

void ARPGAIController::HandleTargetDeath(AActor* DeadActor)
{
	if (TargetCharacter && TargetCharacter == DeadActor)
	{
		ClearTarget();
	}
}

void ARPGAIController::UpdateTargetLocation(const FVector& NewLocation)
{
	if (IsValid(TargetCharacter))
	{
		GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), NewLocation);
	}
} 