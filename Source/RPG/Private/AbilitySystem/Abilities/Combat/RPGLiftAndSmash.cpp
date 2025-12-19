// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/Combat/RPGLiftAndSmash.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Character/RPGEnemy.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "NavigationSystem.h"

// Corpo intencionalmente vazio para recriação da habilidade

void URPGLiftAndSmash::DamageToFloatingTarget(AActor* TargetActor)
{
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("[LiftAndSmash] DamageToFloatingTarget: Target é nulo"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[LiftAndSmash] DamageToFloatingTarget: %s está recebendo dano (stub)"), *TargetActor->GetName());

    ARPGEnemy* Enemy = Cast<ARPGEnemy>(TargetActor);
    if (!Enemy)
    {
        UE_LOG(LogTemp, Warning, TEXT("[LiftAndSmash] Target não é um RPGEnemy. Abortando levitação."));
        return;
    }

    LiftedTarget = Enemy;
    DesiredLiftZ = Enemy->GetActorLocation().Z + LiftHeight;

    // Parar AI/Movimento do inimigo
    LiftedAIController = Enemy->GetController<AAIController>();
    if (LiftedAIController.IsValid())
    {
        bAIStopped = true;
        LiftedAIController->StopMovement();
        // Opcional: pausar lógica de cérebro
        if (UBrainComponent* Brain = LiftedAIController->GetBrainComponent())
        {
            Brain->StopLogic(TEXT("LiftAndSmash - Captured"));
        }
    }

    if (UCharacterMovementComponent* Move = Enemy->GetCharacterMovement())
    {
        // Subida com velocidade controlada (AscentSpeed), ignorando v0 e gravidade
        Move->StopMovementImmediately();
        Move->SetMovementMode(MOVE_Flying);
        const float ClampedSpeed = FMath::Max(10.f, AscentSpeed);
        Move->Velocity = FVector(0.f, 0.f, ClampedSpeed);
        UE_LOG(LogTemp, Log, TEXT("[LiftAndSmash] Subida: Flying, speed=%.1f, alvoZ=%.1f"), ClampedSpeed, DesiredLiftZ);
    }

    if (UWorld* World = GetWorld())
    {
        // Após RiseTime, congela no ar
        World->GetTimerManager().SetTimer(RiseTimerHandle, this, &URPGLiftAndSmash::OnRiseFinished, FMath::Max(0.01f, RiseTime), false);
        // Polling curto para parar precisamente ao atingir a altura
        World->GetTimerManager().SetTimer(RisePollTimerHandle, this, &URPGLiftAndSmash::OnRisePoll, FMath::Max(0.005f, RisePollInterval), true);
    }
}

void URPGLiftAndSmash::OnRiseFinished()
{
    ACharacter* TargetChar = LiftedTarget.Get();
    if (!TargetChar) return;

    if (UCharacterMovementComponent* Move = TargetChar->GetCharacterMovement())
    {
        // Congelar no ar desativando gravidade
        Move->Velocity = FVector::ZeroVector;
        Move->SetMovementMode(MOVE_Falling);
        Move->GravityScale = 0.f;
    }

    // Dispara evento de lock no ar (uma vez) enviando localização
    if (!bHasLockedInAir)
    {
        bHasLockedInAir = true;
        OnTargetLockedInAir.Broadcast(TargetChar, TargetChar->GetActorLocation());
    }

    // Agendar o dano durante o Hang: no início, meio, ou fim, conforme configurado
    if (UWorld* World = GetWorld())
    {
        const float ClampedDamageTime = FMath::Clamp(DamageTimeInHang, 0.f, HangTime);
        World->GetTimerManager().SetTimer(DamageTimerHandle, this, &URPGLiftAndSmash::OnDamageTime, FMath::Max(0.01f, ClampedDamageTime), false);
    }

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RisePollTimerHandle);
        World->GetTimerManager().ClearTimer(RiseTimerHandle);
        // Após HangTime, solta
        World->GetTimerManager().SetTimer(HangTimerHandle, this, &URPGLiftAndSmash::OnHangFinished, FMath::Max(0.01f, HangTime), false);
    }
}

void URPGLiftAndSmash::OnHangFinished()
{
    ACharacter* TargetChar = LiftedTarget.Get();
    if (!TargetChar) return;

    if (UCharacterMovementComponent* Move = TargetChar->GetCharacterMovement())
    {
        // Reativar gravidade e deixar cair
        Move->GravityScale = 1.f;
        Move->SetMovementMode(MOVE_Falling);
        if (DropImpulseZ > 0.f)
        {
            Move->Velocity = FVector(0.f, 0.f, -FMath::Abs(DropImpulseZ));
        }
    }

    CleanupTargetState();
}

void URPGLiftAndSmash::OnDamageTime()
{
    if (ACharacter* TargetChar = LiftedTarget.Get())
    {
        // Broadcast antes/depois conforme necessidade; aqui antes, para sincronizar VFX pré-impacto
        OnDamageApplied.Broadcast(TargetChar, TargetChar->GetActorLocation());
        CauseDamage(TargetChar);
    }
}

void URPGLiftAndSmash::CleanupTargetState()
{
    if (ACharacter* TargetChar = LiftedTarget.Get())
    {
        if (UCharacterMovementComponent* Move = TargetChar->GetCharacterMovement())
        {
            Move->GravityScale = 1.f;
            if (Move->MovementMode == MOVE_None)
            {
                Move->SetMovementMode(MOVE_Falling);
            }
        }
    }
    LiftedTarget.Reset();
    bHasLockedInAir = false;

    // Retomar AI
    if (bAIStopped && LiftedAIController.IsValid())
    {
        if (UBrainComponent* Brain = LiftedAIController->GetBrainComponent())
        {
            Brain->RestartLogic();
        }
        bAIStopped = false;
        LiftedAIController.Reset();
    }
}

void URPGLiftAndSmash::OnRisePoll()
{
    ACharacter* TargetChar = LiftedTarget.Get();
    if (!TargetChar) return;
    const float CurrentZ = TargetChar->GetActorLocation().Z;
    if (CurrentZ >= DesiredLiftZ)
    {
        if (UCharacterMovementComponent* Move = TargetChar->GetCharacterMovement())
        {
            Move->Velocity = FVector::ZeroVector;
        }
        // Clampa exatamente na altura alvo e limpa timers de subida
        const FVector CurrentLoc = TargetChar->GetActorLocation();
        TargetChar->SetActorLocation(FVector(CurrentLoc.X, CurrentLoc.Y, DesiredLiftZ), false, nullptr, ETeleportType::TeleportPhysics);
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(RisePollTimerHandle);
            World->GetTimerManager().ClearTimer(RiseTimerHandle);
        }
        OnRiseFinished();
    }
}
