// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/Magic/RPGRainOfLightAbility.h"
#include "Engine/Engine.h"
#include "Character/RPGEnemy.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

URPGRainOfLightAbility::URPGRainOfLightAbility()
{
}

void URPGRainOfLightAbility::ExecuteRainOfLight(AActor* Target)
{
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rain of Light - Target is null!"));
		return;
	}

	ARPGEnemy* EnemyTarget = Cast<ARPGEnemy>(Target);
	if (!EnemyTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rain of Light - Target is not a valid RPGEnemy! Target: %s"), *Target->GetName());
		return;
	}

	// 1) Log do alvo e detecção ao redor
	UE_LOG(LogTemp, Log, TEXT("Rain of Light - Execute on Target: %s at %s"), *Target->GetName(), *Target->GetActorLocation().ToString());
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	TArray<AActor*> ActorsToIgnore; ActorsToIgnore.Add(Target);
	TArray<AActor*> OutActors;

	const FVector Center = Target->GetActorLocation();
	const bool bFound = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(), Center, DetectionRadius, ObjectTypes, nullptr, ActorsToIgnore, OutActors);

	UE_LOG(LogTemp, Log, TEXT("Rain of Light - Sphere detect radius=%.1f found=%d"), DetectionRadius, bFound ? OutActors.Num() : 0);
	if (bFound)
	{
		for (AActor* Actor : OutActors)
		{
			UE_LOG(LogTemp, Log, TEXT("Rain of Light - Detected: %s at %s"), *Actor->GetName(), *Actor->GetActorLocation().ToString());
		}
	}

	// 2) Aplicar o sistema completo no alvo central
	ApplyStunToEnemy(EnemyTarget);

	// 2.1) Causar dano no alvo central
	UE_LOG(LogTemp, Log, TEXT("Rain of Light - Causing damage to central target: %s"), *EnemyTarget->GetName());
	CauseDamage(EnemyTarget);

	// 3) Aplicar apenas o stun nos demais detectados
	if (bFound)
	{
		for (AActor* Actor : OutActors)
		{
			if (ARPGEnemy* Satellite = Cast<ARPGEnemy>(Actor))
			{
				ApplySatelliteStun(Satellite);
				// Causar dano também nos satélites
				UE_LOG(LogTemp, Log, TEXT("Rain of Light - Causing damage to satellite: %s"), *Satellite->GetName());
				CauseDamage(Satellite);
			}
		}
	}
}

bool URPGRainOfLightAbility::GetGroundLocationUnderActor(const AActor* Actor, FVector& OutLocation, FVector& OutNormal) const
{
	OutLocation = FVector::ZeroVector;
	OutNormal = FVector::UpVector;
	if (!Actor) return false;

	const FVector Start = Actor->GetActorLocation() + FVector(0.f, 0.f, 50.f);
	const FVector End = Start - FVector(0.f, 0.f, 2000.f);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
	TArray<AActor*> ActorsToIgnore;
	FHitResult Hit;

	const bool bHit = UKismetSystemLibrary::LineTraceSingleForObjects(
		GetWorld(), Start, End, ObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true);

	if (bHit && Hit.bBlockingHit)
	{
		OutLocation = Hit.Location;
		OutNormal = Hit.Normal;
		return true;
	}
	return false;
}

void URPGRainOfLightAbility::ApplyStunToEnemy(ARPGEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	// Salvar referências fracas (alvo central)
	StunnedEnemy = Enemy;
	EnemyAIController = Enemy->GetController<AAIController>();

	// Parar movimento do inimigo
	if (UCharacterMovementComponent* Move = Enemy->GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->SetMovementMode(MOVE_None); // Congelar completamente
	}

	// Parar AI do inimigo
	if (AAIController* AI = EnemyAIController.Get())
	{
		AI->StopMovement();
		if (UBrainComponent* Brain = AI->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Rain of Light - Stunned"));
		}
	}

	// Calcular localização do chão sob o inimigo e emitir evento
	FVector GroundLocation; FVector GroundNormal;
	if (GetGroundLocationUnderActor(Enemy, GroundLocation, GroundNormal))
	{
		OnTargetStunned.Broadcast(Enemy, GroundLocation);
		if (bSpawnGroundNiagara && GroundNiagaraSystem)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(), GroundNiagaraSystem, GroundLocation, GroundNormal.Rotation(), GroundNiagaraScale, true, true, ENCPoolMethod::AutoRelease);
		}
	}
	else
	{
		OnTargetStunned.Broadcast(Enemy, Enemy->GetActorLocation());
	}

	// Configurar timer para remover o stun do alvo central
	if (UWorld* World = GetWorld())
	{
		if (StunTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(StunTimerHandle);
		}
		World->GetTimerManager().SetTimer(
			StunTimerHandle,
			FTimerDelegate::CreateUObject(this, &URPGRainOfLightAbility::RemoveStun),
			FMath::Max(0.01f, StunDuration),
			false
		);
	}
}

void URPGRainOfLightAbility::ApplySatelliteStun(ARPGEnemy* Enemy)
{
	if (!Enemy)
	{
		return;
	}

	// Já está stunnado como central?
	if (StunnedEnemy.IsValid() && StunnedEnemy.Get() == Enemy)
	{
		return;
	}

	// Se já possui timer de satélite, reinicia
	if (FTimerHandle* ExistingHandle = SatelliteStunTimers.Find(Enemy))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(*ExistingHandle);
			World->GetTimerManager().SetTimer(
				*ExistingHandle,
				FTimerDelegate::CreateWeakLambda(this, [this, WeakEnemy = TWeakObjectPtr<ARPGEnemy>(Enemy)]()
				{
					if (ARPGEnemy* E = WeakEnemy.Get())
					{
						RemoveSatelliteStun(E);
					}
				}),
				FMath::Max(0.01f, StunDuration),
				false
			);
		}
	}
	else
	{
		// Aplicar restrições (movimento + AI) sem efeitos/evento
		if (UCharacterMovementComponent* Move = Enemy->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
			Move->SetMovementMode(MOVE_None);
		}
		if (AAIController* AI = Enemy->GetController<AAIController>())
		{
			AI->StopMovement();
			if (UBrainComponent* Brain = AI->GetBrainComponent())
			{
				Brain->StopLogic(TEXT("Rain of Light - Stunned (Satellite)"));
			}
		}

		// Criar e iniciar timer próprio de satélite
		FTimerHandle Handle;
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				Handle,
				FTimerDelegate::CreateWeakLambda(this, [this, WeakEnemy = TWeakObjectPtr<ARPGEnemy>(Enemy)]()
				{
					if (ARPGEnemy* E = WeakEnemy.Get())
					{
						RemoveSatelliteStun(E);
					}
				}),
				FMath::Max(0.01f, StunDuration),
				false
			);
		}
		SatelliteStunTimers.Add(Enemy, Handle);
	}
}

void URPGRainOfLightAbility::RemoveSatelliteStun(ARPGEnemy* Enemy)
{
	if (!Enemy) return;
	FTimerHandle Handle;
	if (SatelliteStunTimers.RemoveAndCopyValue(Enemy, Handle))
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(Handle);
		}
	}

	if (UCharacterMovementComponent* Move = Enemy->GetCharacterMovement())
	{
		if (Move->MovementMode == MOVE_None)
		{
			Move->SetMovementMode(MOVE_Walking);
		}
	}
	if (AAIController* AI = Enemy->GetController<AAIController>())
	{
		if (UBrainComponent* Brain = AI->GetBrainComponent())
		{
			Brain->RestartLogic();
		}
	}
}

void URPGRainOfLightAbility::RemoveAllSatelliteStuns()
{
	TArray<TWeakObjectPtr<ARPGEnemy>> Keys;
	SatelliteStunTimers.GetKeys(Keys);
	for (const TWeakObjectPtr<ARPGEnemy>& WeakEnemy : Keys)
	{
		if (ARPGEnemy* Enemy = WeakEnemy.Get())
		{
			RemoveSatelliteStun(Enemy);
		}
	}
	SatelliteStunTimers.Empty();
}

void URPGRainOfLightAbility::RemoveStun()
{
	ARPGEnemy* Enemy = StunnedEnemy.Get();
	AAIController* AI = EnemyAIController.Get();

	if (Enemy)
	{
		if (UCharacterMovementComponent* Move = Enemy->GetCharacterMovement())
		{
			if (Move->MovementMode == MOVE_None)
			{
				Move->SetMovementMode(MOVE_Walking);
			}
		}
	}

	if (AI)
	{
		if (UBrainComponent* Brain = AI->GetBrainComponent())
		{
			Brain->RestartLogic();
		}
	}

	if (UWorld* World = GetWorld())
	{
		if (StunTimerHandle.IsValid())
		{
			World->GetTimerManager().ClearTimer(StunTimerHandle);
		}
	}

	StunnedEnemy = nullptr;
	EnemyAIController = nullptr;
}

void URPGRainOfLightAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
									  const FGameplayAbilityActorInfo* ActorInfo,
									  const FGameplayAbilityActivationInfo ActivationInfo,
									  bool bReplicateEndAbility,
									  bool bWasCancelled)
{
	RemoveStun();
	RemoveAllSatelliteStuns();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
