// Copyright Druid Mechanics


#include "AbilitySystem/Tasks/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"
#include "RPG/RPG.h"
#include "Character/RPGCharacter.h"
#include "DrawDebugHelpers.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
    if (bIsLocallyControlled)
	{
		SendMouseCursorData();
	}
	else
	{
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	// Trace a partir do personagem (não da câmera)
	ARPGCharacter* RPGChar = Cast<ARPGCharacter>(Ability->GetCurrentActorInfo()->AvatarActor);
	if (!RPGChar)
	{
		UE_LOG(LogTemp, Error, TEXT("[TargetDataUnderMouse] AvatarActor não é um RPGCharacter"));
		return;
	}
	
	// Backstop removido - não é mais necessário com o trace da frente da câmera
	
	// Obter direção da câmera para manter o targeting baseado no crosshair
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FVector CameraLocation;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(CameraLocation, ViewRot);
	
	// Usar direção da câmera e começar um pouco na frente da câmera
	FVector Start = CameraLocation + ViewRot.Vector() * 100.f; // 100cm na frente da câmera
	FVector Direction = ViewRot.Vector();
	
	// Usar MaxTraceRange configurado ou o padrão do personagem
	float MaxRange = (MaxTraceRange > 0.f) ? MaxTraceRange : RPGChar->MaxTargetingRange;
	
	// Calcular ponto de destino
	FVector End = Start + Direction * MaxRange;
	
	// Trace simples - sempre vai acertar algo (objeto real ou backstop)
	FCollisionQueryParams Params(NAME_None, false, nullptr); // Comentado PC->GetPawn() para teste
	FHitResult CursorHit;
	GetWorld()->LineTraceSingleByChannel(CursorHit, Start, End, ECC_Visibility, Params);
	
	// Debug visual do trace
	if (bDrawDebugTrace)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			FColor TraceColor = CursorHit.bBlockingHit ? FColor::Green : FColor::Red;
			float DebugDuration = 2.0f;
			float DebugThickness = 2.0f;
			
			// Desenhar linha do trace
			DrawDebugLine(World, Start, End, TraceColor, false, DebugDuration, 0, DebugThickness);
			
			// Desenhar esfera no ponto de início
			DrawDebugSphere(World, Start, 5.0f, 8, TraceColor, false, DebugDuration, 0, DebugThickness);
			
			// Se acertou algo, desenhar esfera no ponto de impacto
			if (CursorHit.bBlockingHit)
			{
				DrawDebugSphere(World, CursorHit.ImpactPoint, 10.0f, 12, FColor::Yellow, false, DebugDuration, 0, DebugThickness);
				// Desenhar linha normal
				DrawDebugLine(World, CursorHit.ImpactPoint, CursorHit.ImpactPoint + CursorHit.Normal * 50.0f, FColor::Blue, false, DebugDuration, 0, DebugThickness);
			}
			else
			{
				// Desenhar esfera no ponto final se não acertou nada
				DrawDebugSphere(World, End, 5.0f, 8, TraceColor, false, DebugDuration, 0, DebugThickness);
			}
		}
	}
	
	// Log do resultado do trace
	if (CursorHit.bBlockingHit)
	{
		UE_LOG(LogTemp, Log, TEXT("[TargetDataUnderMouse] Hit - acertou: %s em (%.1f, %.1f, %.1f)"), 
			*GetNameSafe(CursorHit.GetActor()), 
			CursorHit.ImpactPoint.X, CursorHit.ImpactPoint.Y, CursorHit.ImpactPoint.Z);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[TargetDataUnderMouse] Miss - não acertou nada"));
	}

	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);
	
    if (bReplicateTargetData)
    {
        AbilitySystemComponent->ServerSetReplicatedTargetData(
            GetAbilitySpecHandle(),
            GetActivationPredictionKey(),
            DataHandle,
            FGameplayTag(),
            AbilitySystemComponent->ScopedPredictionKey);
    }

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
