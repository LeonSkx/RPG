// Copyright Druid Mechanics

#include "AI/RPGPartyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
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
#include "Party/PartySubsystem.h"


ARPGPartyAIController::ARPGPartyAIController()
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

void ARPGPartyAIController::SetTarget(ARPGCharacterBase* NewTarget)
{
	// Só aceita inimigos como alvo
	ARPGEnemy* EnemyTarget = Cast<ARPGEnemy>(NewTarget);
	if (!EnemyTarget) return;

	if (IsValid(TargetCharacter))
	{
		TargetCharacter->GetOnDeathDelegate().RemoveAll(this);
	}

	TargetCharacter = EnemyTarget;

	if (IsValid(TargetCharacter))
	{
		TargetCharacter->GetOnDeathDelegate().AddDynamic(this, &ARPGPartyAIController::HandleTargetDeath);
	}
	
	GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), EnemyTarget);

	// Atualizar o CombatTarget do membro da party com o novo alvo usando a interface
	if (ARPGCharacter* PartyMember = Cast<ARPGCharacter>(GetPawn()))
	{
		if (PartyMember->GetClass()->ImplementsInterface(UPartyInterface::StaticClass()))
		{
			IPartyInterface::Execute_SetCombatTarget(PartyMember, EnemyTarget);
		}
		// Se há um alvo válido, atualizar a localização conhecida
		if (IsValid(EnemyTarget))
		{
			GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), EnemyTarget->GetActorLocation());
		}
	}
}

void ARPGPartyAIController::ClearTarget()
{
	if (IsValid(TargetCharacter))
	{
		TargetCharacter->GetOnDeathDelegate().RemoveAll(this);
	}
	TargetCharacter = nullptr;
	GetBlackboardComponent()->ClearValue(TEXT("TargetActor"));

	// Limpar o CombatTarget do membro da party usando a interface
	if (ARPGCharacter* PartyMember = Cast<ARPGCharacter>(GetPawn()))
	{
		if (PartyMember->GetClass()->ImplementsInterface(UPartyInterface::StaticClass()))
		{
			IPartyInterface::Execute_SetCombatTarget(PartyMember, nullptr);
		}
	}
}

void ARPGPartyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	GetAIPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ARPGPartyAIController::OnTargetPerceptionUpdated);
	
	// Configurar timer para verificar se está seguro e atualizar PartyMember
	GetWorld()->GetTimerManager().SetTimer(
		UpdatePartyMemberTimerHandle,
		FTimerDelegate::CreateUObject(this, &ARPGPartyAIController::UpdatePartyMemberWhenSafe),
		0.2f, // Verificar a cada 0.2 segundos (mais frequente)
		true   // Repetir
	);

    // Vincular aos eventos do PartySubsystem para acompanhar o membro ativo
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                if (!bBoundToPartySubsystem)
                {
                    PartySubsystem->OnActivePartyMemberChanged.AddDynamic(this, &ARPGPartyAIController::HandleActiveMemberChanged);
                    bBoundToPartySubsystem = true;
                }
            }
        }
    }

    // Forçar alinhamento inicial com o estado da party
    RefreshActiveState();

    // Vincular ao evento de morte do próprio pawn para parar a BT
    if (ARPGCharacterBase* OwnerChar = Cast<ARPGCharacterBase>(InPawn))
    {
        OwnerChar->GetOnDeathDelegate().AddDynamic(this, &ARPGPartyAIController::HandleOwnerDeath);
    }
}

void ARPGPartyAIController::OnUnPossess()
{
    Super::OnUnPossess();
    // Pausar lógica de BT quando não estiver controlando ninguém
    if (UBrainComponent* Brain = BrainComponent)
    {
        Brain->StopLogic(TEXT("UnPossess"));
    }

    // Desvincular do evento de morte do pawn anterior (se possível)
    if (APawn* PreviousPawn = GetPawn())
    {
        if (ARPGCharacterBase* OwnerChar = Cast<ARPGCharacterBase>(PreviousPawn))
        {
            OwnerChar->GetOnDeathDelegate().RemoveAll(this);
        }
    }
}

void ARPGPartyAIController::RefreshActiveState()
{
    UWorld* World = GetWorld();
    if (!World) return;
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance) return;
    UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>();
    if (!PartySubsystem) return;

    ARPGCharacter* Active = PartySubsystem->GetActivePartyMember();
    APawn* Controlled = GetPawn();
    const bool bShouldBeAI = Controlled && Controlled != Active;

    if (bShouldBeAI)
    {
        // Garantir que a lógica de BT esteja rodando
        if (ARPGCharacter* PartyMember = Cast<ARPGCharacter>(Controlled))
        {
            if (UBehaviorTree* BT = PartyMember->GetBehaviorTree())
            {
                if (UBlackboardComponent* BB = GetBlackboardComponent())
                {
                    if (!BB->GetBlackboardAsset())
                    {
                        BB->InitializeBlackboard(*BT->BlackboardAsset);
                    }
                }
                RunBehaviorTree(BT);
            }
        }
    }
    else
    {
        // Se por algum motivo estivermos controlando o ativo, pausar lógica para não conflitar com o PlayerController
        if (UBrainComponent* Brain = BrainComponent)
        {
            Brain->StopLogic(TEXT("IsActivePlayer"));
        }
    }
}

void ARPGPartyAIController::HandleActiveMemberChanged(ARPGCharacter* NewActive)
{
    // Reavaliar se devemos rodar/parar a BT baseado no novo ativo
    RefreshActiveState();
}

void ARPGPartyAIController::HandleOwnerDeath(AActor* DeadActor)
{
    // Confirmar que o morto é o nosso próprio pawn atual
    if (DeadActor != GetPawn())
    {
        return;
    }

    // Parar completamente a BT/Brain para não executar mais tasks
    if (UBehaviorTreeComponent* BTC = Cast<UBehaviorTreeComponent>(BrainComponent))
    {
        BTC->StopTree(EBTStopMode::Forced);
    }
    if (UBrainComponent* Brain = BrainComponent)
    {
        Brain->StopLogic(TEXT("OwnerDead"));
    }

    // Limpar dados do blackboard relevantes
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->ClearValue(TEXT("TargetActor"));
        BB->ClearValue(TEXT("LastKnownLocation"));
        BB->ClearValue(TEXT("NoiseLocation"));
        BB->ClearValue(TEXT("PartyMember"));
        BB->ClearValue(TEXT("ActivePlayerLocation"));
    }
}

void ARPGPartyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;
	
	// Verificar se o Blackboard está inicializado
	if (!GetBlackboardComponent() || !GetBlackboardComponent()->GetBlackboardAsset())
	{
		return;
	}

	// --- DANO: Só processa se for inimigo ---
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		ARPGEnemy* EnemyTarget = Cast<ARPGEnemy>(Actor);
		if (!EnemyTarget) return;
		
		if (!IsValid(TargetCharacter))
		{
			const float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), EnemyTarget->GetActorLocation());
			const float MaxDamageResponseDistance = 1000.0f;
			if (DistanceToTarget <= MaxDamageResponseDistance)
			{
				SetTarget(EnemyTarget);
			}
		}
		else
		{
			GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), EnemyTarget->GetActorLocation());
		}
		return;
	}

	// --- SOM: Nunca seta alvo, só atualiza localização e limpa PartyMember ---
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		// Verificar se o som vem de um membro da party
		ARPGCharacter* PartyMember = Cast<ARPGCharacter>(Actor);
		if (PartyMember)
		{
			// Ignorar sons dos membros da party
			return;
		}
		
		GetBlackboardComponent()->SetValueAsVector(TEXT("NoiseLocation"), Stimulus.StimulusLocation);
		
		// Limpar PartyMember quando ouvir som (ficar alerta)
		GetBlackboardComponent()->ClearValue(TEXT("PartyMember"));
		GetBlackboardComponent()->ClearValue(TEXT("ActivePlayerLocation"));
		
		return;
	}

	// --- VISÃO: Só processa se for inimigo ---
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		ARPGEnemy* EnemyTarget = Cast<ARPGEnemy>(Actor);
		if (!EnemyTarget) return;
		
		const bool bIsHostile = GetTeamAttitudeTowards(*EnemyTarget) == ETeamAttitude::Hostile;
		if (!bIsHostile) return;
		
		if (Stimulus.WasSuccessfullySensed())
		{
			if (!IsValid(TargetCharacter))
			{
				SetTarget(EnemyTarget);
			}
			else if (TargetCharacter == EnemyTarget)
			{
				GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), EnemyTarget->GetActorLocation());
			}
		}
		else // Alvo de visão perdido
		{
			if (TargetCharacter == EnemyTarget)
			{
				GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), TargetCharacter->GetActorLocation());
				ClearTarget();
				
				// Limpar todas as outras chaves e setar PartyMember imediatamente
				GetBlackboardComponent()->ClearValue(TEXT("NoiseLocation"));
				GetBlackboardComponent()->ClearValue(TEXT("TargetActor"));
				
				// Atualizar PartyMember imediatamente
				UPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<UPartySubsystem>();
				if (PartySubsystem)
				{
					ARPGCharacter* ActiveCharacter = PartySubsystem->GetActivePartyMember();
					if (IsValid(ActiveCharacter))
					{
						GetBlackboardComponent()->SetValueAsObject(TEXT("PartyMember"), ActiveCharacter);
						GetBlackboardComponent()->SetValueAsVector(TEXT("ActivePlayerLocation"), ActiveCharacter->GetActorLocation());
					}
				}
			}
		}
		return;
	}

	// --- Outros estímulos: ignorar ---
}

void ARPGPartyAIController::UpdatePartyMemberWhenSafe()
{
	// Verificar se o Blackboard está inicializado
	if (!GetBlackboardComponent() || !GetBlackboardComponent()->GetBlackboardAsset())
	{
		return;
	}
	
	// Log removed for cleanup
	
	// Só atualizar PartyMember se estiver completamente em paz
	if (IsValid(TargetCharacter)) 
	{
		// Verificar se o alvo está morto usando a interface
		ARPGEnemy* EnemyTarget = Cast<ARPGEnemy>(TargetCharacter);
		if (EnemyTarget && ICombatInterface::Execute_IsDead(EnemyTarget))
		{
			// Se o alvo está morto, limpar tudo e setar PartyMember
			ClearTarget();
			GetBlackboardComponent()->ClearValue(TEXT("NoiseLocation"));
			GetBlackboardComponent()->ClearValue(TEXT("TargetActor"));
			
			// Atualizar PartyMember imediatamente
			UPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<UPartySubsystem>();
			if (PartySubsystem)
			{
				ARPGCharacter* ActiveCharacter = PartySubsystem->GetActivePartyMember();
				if (IsValid(ActiveCharacter))
				{
					GetBlackboardComponent()->SetValueAsObject(TEXT("PartyMember"), ActiveCharacter);
					GetBlackboardComponent()->SetValueAsVector(TEXT("ActivePlayerLocation"), ActiveCharacter->GetActorLocation());
				}
			}
			return;
		}
		
		// Se tem alvo vivo, limpar PartyMember
		GetBlackboardComponent()->ClearValue(TEXT("PartyMember"));
		GetBlackboardComponent()->ClearValue(TEXT("ActivePlayerLocation"));
		return;
	}
	
	// Se chegou até aqui, está em paz - atualizar PartyMember
	UPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<UPartySubsystem>();
	if (!PartySubsystem) 
	{
		return;
	}
	
	ARPGCharacter* ActiveCharacter = PartySubsystem->GetActivePartyMember();
	if (IsValid(ActiveCharacter))
	{
		// Setar o player ativo como PartyMember no blackboard
		GetBlackboardComponent()->SetValueAsObject(TEXT("PartyMember"), ActiveCharacter);
		// Atualizar a localização do player ativo
		GetBlackboardComponent()->SetValueAsVector(TEXT("ActivePlayerLocation"), ActiveCharacter->GetActorLocation());
	}
	else
	{
		// Limpar se não há player ativo
		GetBlackboardComponent()->ClearValue(TEXT("PartyMember"));
		GetBlackboardComponent()->ClearValue(TEXT("ActivePlayerLocation"));
	}
}

void ARPGPartyAIController::HandleTargetDeath(AActor* DeadActor)
{
	// Verificar se o Blackboard está inicializado
	if (!GetBlackboardComponent() || !GetBlackboardComponent()->GetBlackboardAsset())
	{
		return;
	}
	
	// Log removed for cleanup
	
	// Verificar se o alvo está realmente morto usando a interface
	ARPGEnemy* EnemyTarget = Cast<ARPGEnemy>(DeadActor);
	if (EnemyTarget && ICombatInterface::Execute_IsDead(EnemyTarget))
	{
		// Limpar todas as outras chaves e setar PartyMember imediatamente
		GetBlackboardComponent()->ClearValue(TEXT("NoiseLocation"));
		GetBlackboardComponent()->ClearValue(TEXT("TargetActor"));
		
		// Atualizar PartyMember imediatamente
		UPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<UPartySubsystem>();
		if (PartySubsystem)
		{
			ARPGCharacter* ActiveCharacter = PartySubsystem->GetActivePartyMember();
			if (IsValid(ActiveCharacter))
			{
				GetBlackboardComponent()->SetValueAsObject(TEXT("PartyMember"), ActiveCharacter);
				GetBlackboardComponent()->SetValueAsVector(TEXT("ActivePlayerLocation"), ActiveCharacter->GetActorLocation());
			}
		}
		
		// Limpar o alvo por último para evitar conflitos
		ClearTarget();
	}
	// Log removed for cleanup
}

void ARPGPartyAIController::UpdateTargetLocation(const FVector& NewLocation)
{
	if (IsValid(TargetCharacter))
	{
		GetBlackboardComponent()->SetValueAsVector(TEXT("LastKnownLocation"), NewLocation);
	}
} 