// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/Magic/RPGTeleportAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"

URPGTeleportAbility::URPGTeleportAbility()
{
	// Configurações padrão
	TeleportDistance = 300.f;
	MaxTeleportHeight = 100.f;
	CollisionCheckRadius = 50.f;
	MinTeleportDistance = 100.f;
	GroundCheckDistance = 200.f;
	MaxTeleportAttempts = 3;
	bRequireGroundAtDestination = false; // Desabilitado por padrão
	bUseMovementInputDirection = true;
	bUseCameraDirection = false; // Desabilitado por padrão
	bRequireMovement = true; // Exige movimento por padrão
	MovementThreshold = 10.f; // Velocidade mínima para considerar movimento
	
	// Debug: Log das configurações iniciais
	UE_LOG(LogTemp, Log, TEXT("RPGTeleportAbility Constructor - MovementThreshold: %.2f"), MovementThreshold);
}

void URPGTeleportAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
										 const FGameplayAbilityActorInfo* ActorInfo,
										 const FGameplayAbilityActivationInfo ActivationInfo,
										 const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// Não executa o teleport automaticamente - aguarda chamada do Blueprint
	// O Blueprint deve chamar ActivateTeleport() quando quiser
}

bool URPGTeleportAbility::TryTeleport(const FVector& Direction)
{
    // Verifica se a habilidade está ativa
    if (!IsActive())
    {
        return false;
    }
    
    return ExecuteTeleport(Direction);
}

void URPGTeleportAbility::ActivateTeleport()
{
	UE_LOG(LogTemp, Log, TEXT("ActivateTeleport - bRequireMovement: %s"), bRequireMovement ? TEXT("true") : TEXT("false"));
	
	// Obtém a direção do input
	FVector InputDirection = GetInputDirection();
	UE_LOG(LogTemp, Log, TEXT("Input Direction: %s"), *InputDirection.ToString());
	
	// Verifica se há direção válida
	if (InputDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("Teleport Failed - No valid direction"));
		OnTeleportFailed();
		return;
	}
	
	// Tenta executar o teleport
	if (!ExecuteTeleport(InputDirection))
	{
		if (!TryAlternativeDirections(InputDirection))
		{
			UE_LOG(LogTemp, Warning, TEXT("Teleport Failed - All attempts failed"));
			OnTeleportFailed();
		}
	}
}

bool URPGTeleportAbility::ExecuteTeleport(const FVector& Direction)
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return false;
	}

	// Calcula a posição de destino
	FVector Destination = CalculateTeleportDestination(Direction);
	
	// Valida e ajusta a posição se necessário
	FVector ValidDestination;
	if (!FindValidTeleportDestination(Destination, ValidDestination))
	{
		return false;
	}

	// Executa o teleporte
	FVector OldLocation = AvatarActor->GetActorLocation();
	bool bTeleportSuccess = AvatarActor->SetActorLocation(ValidDestination, true);
	
	if (bTeleportSuccess)
	{
		OnTeleportSuccess(OldLocation, ValidDestination);
		return true;
	}

	return false;
}

bool URPGTeleportAbility::TryAlternativeDirections(const FVector& OriginalDirection)
{
	// Tenta direções em ângulos de 45 graus
	TArray<float> Angles = {45.f, -45.f, 90.f, -90.f, 135.f, -135.f};
	
	for (float Angle : Angles)
	{
		FVector RotatedDirection = OriginalDirection.RotateAngleAxis(Angle, FVector::UpVector);
		if (ExecuteTeleport(RotatedDirection))
		{
			return true;
		}
	}
	
	return false;
}

FVector URPGTeleportAbility::CalculateTeleportDestination(const FVector& Direction) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return FVector::ZeroVector;
	}

	FVector CurrentLocation = AvatarActor->GetActorLocation();
	FVector NormalizedDirection = Direction.GetSafeNormal();
	
	// Calcula múltiplas distâncias para tentar encontrar um local válido
	TArray<float> DistancesToTry = {TeleportDistance, TeleportDistance * 0.75f, TeleportDistance * 0.5f};
	
	for (float Distance : DistancesToTry)
	{
		if (Distance >= MinTeleportDistance)
		{
			FVector TestDestination = CurrentLocation + (NormalizedDirection * Distance);
			TestDestination.Z = CurrentLocation.Z; // Mantém a altura inicial
			
			FVector ValidDestination;
			if (FindValidTeleportDestination(TestDestination, ValidDestination))
			{
				return ValidDestination;
			}
		}
	}
	
	// Se nenhuma distância funcionou, retorna a tentativa original
	return CurrentLocation + (NormalizedDirection * TeleportDistance);
}

bool URPGTeleportAbility::FindValidTeleportDestination(const FVector& DesiredDestination, FVector& OutValidDestination) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	
	if (!AvatarActor || !World)
	{
		return false;
	}

	FVector TestDestination = DesiredDestination;
	
	// 1. Verifica colisão no ponto de destino
	if (!IsLocationClear(TestDestination))
	{
		return false;
	}

	// 2. Verifica altura máxima
	FVector CurrentLocation = AvatarActor->GetActorLocation();
	float HeightDifference = FMath::Abs(TestDestination.Z - CurrentLocation.Z);
	if (HeightDifference > MaxTeleportHeight)
	{
		return false;
	}

	// 3. Verifica se há chão no destino (se necessário)
	if (bRequireGroundAtDestination)
	{
		FVector GroundLocation;
		if (!FindGroundAtLocation(TestDestination, GroundLocation))
		{
			return false;
		}
		TestDestination = GroundLocation;
	}

	// 4. Verificação final de colisão na posição ajustada
	if (!IsLocationClear(TestDestination))
	{
		return false;
	}

	OutValidDestination = TestDestination;
	return true;
}

bool URPGTeleportAbility::IsLocationClear(const FVector& Location) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	UWorld* World = GetWorld();
	
	if (!AvatarActor || !World)
	{
		return false;
	}

	// Usa o raio da cápsula do personagem se disponível
	float CheckRadius = CollisionCheckRadius;
	if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
	{
		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			CheckRadius = FMath::Max(CheckRadius, Capsule->GetScaledCapsuleRadius());
		}
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);
	QueryParams.bTraceComplex = false;
	
	FHitResult HitResult;
	bool bHit = World->SweepSingleByChannel(
		HitResult,
		Location,
		Location,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(CheckRadius),
		QueryParams
	);

	return !bHit;
}

bool URPGTeleportAbility::FindGroundAtLocation(const FVector& Location, FVector& OutGroundLocation) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector TraceStart = Location + FVector(0, 0, 50.f);
	FVector TraceEnd = Location - FVector(0, 0, GroundCheckDistance);
	
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	
	FHitResult HitResult;
	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHit && HitResult.bBlockingHit)
	{
		// Ajusta a posição para ficar ligeiramente acima do chão
		OutGroundLocation = HitResult.Location + FVector(0, 0, 5.f);
		return true;
	}

	return false;
}

FVector URPGTeleportAbility::GetInputDirection() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return FVector::ZeroVector;
	}

	FVector InputDirection = FVector::ZeroVector;

	// Debug: Log das configurações
	UE_LOG(LogTemp, Log, TEXT("Teleport Config - bUseMovementInputDirection: %s, bUseCameraDirection: %s, MovementThreshold: %.2f"), 
		bUseMovementInputDirection ? TEXT("true") : TEXT("false"),
		bUseCameraDirection ? TEXT("true") : TEXT("false"),
		MovementThreshold);

	// 1. Verifica se o personagem está se movendo (velocidade atual)
	if (bUseMovementInputDirection)
	{
		if (ACharacter* Character = Cast<ACharacter>(AvatarActor))
		{
			// Verifica a velocidade atual do personagem
			FVector CurrentVelocity = Character->GetVelocity();
			CurrentVelocity.Z = 0.f; // Ignora movimento vertical
			
			float VelocitySize = CurrentVelocity.Size();
			UE_LOG(LogTemp, Log, TEXT("Current Velocity Size: %.2f, Threshold: %.2f"), VelocitySize, MovementThreshold);
			
			// Se está se movendo com velocidade significativa
			if (VelocitySize > MovementThreshold)
			{
				InputDirection = CurrentVelocity.GetSafeNormal();
				UE_LOG(LogTemp, Log, TEXT("Using Movement Direction: %s"), *InputDirection.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("Velocity too low, not using movement direction"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Movement input direction disabled"));
	}

	// 2. Se não está se movendo, tenta usar direção da câmera (se habilitado)
	if (InputDirection.IsNearlyZero() && bUseCameraDirection)
	{
		if (APlayerController* PC = Cast<APlayerController>(AvatarActor->GetInstigatorController()))
		{
			FRotator ControllerRotation = PC->GetControlRotation();
			ControllerRotation.Pitch = 0.f;
			ControllerRotation.Roll = 0.f;
			InputDirection = FRotationMatrix(ControllerRotation).GetUnitAxis(EAxis::X);
			UE_LOG(LogTemp, Log, TEXT("Using Camera Direction: %s"), *InputDirection.ToString());
		}
	}
	else if (InputDirection.IsNearlyZero() && !bUseCameraDirection)
	{
		UE_LOG(LogTemp, Log, TEXT("Camera direction disabled"));
	}

	// 3. Se não há direção, retorna zero (sem fallback)
	if (InputDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Log, TEXT("No valid direction found - returning zero"));
	}

	return InputDirection;
}

void URPGTeleportAbility::OnTeleportSuccess(const FVector& OldLocation, const FVector& NewLocation)
{
	// Override este método nas classes filhas para adicionar efeitos
	// Exemplo: spawn de partículas, sons, screen shake, etc.
	
	// Log para debug
	UE_LOG(LogTemp, Log, TEXT("Teleport Success: %s -> %s"), 
		*OldLocation.ToString(), *NewLocation.ToString());
}

void URPGTeleportAbility::OnTeleportFailed()
{
	// Override este método nas classes filhas para feedback de falha
	// Exemplo: som de erro, shake da câmera, etc.
	
	UE_LOG(LogTemp, Warning, TEXT("Teleport Failed"));
} 