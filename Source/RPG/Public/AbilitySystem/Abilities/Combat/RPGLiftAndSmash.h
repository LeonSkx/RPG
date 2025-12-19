// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Base/RPGDamageGameplayAbility.h"
#include "RPGLiftAndSmash.generated.h"

class AActor;

UCLASS()
class RPG_API URPGLiftAndSmash : public URPGDamageGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lift")
	void DamageToFloatingTarget(AActor* TargetActor);

	// Evento: alvo travado no ar (envia o alvo e a localização no ar)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLiftTargetLockedInAirSignature, AActor*, TargetActor, FVector, AirLocation);

	UPROPERTY(BlueprintAssignable, Category = "Lift|Events")
	FLiftTargetLockedInAirSignature OnTargetLockedInAir;

	// Evento: dano aplicado (envia o alvo e a localização no ar no instante do dano)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLiftDamageAppliedSignature, AActor*, TargetActor, FVector, AirLocation);

	UPROPERTY(BlueprintAssignable, Category = "Lift|Events")
	FLiftDamageAppliedSignature OnDamageApplied;

protected:
	// Velocidade inicial para lançar o alvo para cima (eixo Z)
	UPROPERTY(EditDefaultsOnly, Category = "Lift")
	float LiftVelocityZ = 1200.f;

	// Tempo até atingir o pico (segundos)
	UPROPERTY(EditDefaultsOnly, Category = "Lift")
	float RiseTime = 0.35f;

	// Velocidade constante de subida em cm/s (se usada, ignora o cálculo de v0)
	UPROPERTY(EditDefaultsOnly, Category = "Lift")
	float AscentSpeed = 300.f;

	// Altura que o alvo deve alcançar a partir da posição atual (em centímetros)
	UPROPERTY(EditDefaultsOnly, Category = "Lift")
	float LiftHeight = 600.f;

	// Tempo que fica parado no ar (segundos)
	UPROPERTY(EditDefaultsOnly, Category = "Lift")
	float HangTime = 0.6f;

	// Segundo dentro do HangTime em que o dano é aplicado (0 = início, >= HangTime = fim)
	UPROPERTY(EditDefaultsOnly, Category = "Lift|DamageTiming")
	float DamageTimeInHang = 0.f;

	// Impulso negativo quando soltar (0 = não forçar queda)
	UPROPERTY(EditDefaultsOnly, Category = "Lift")
	float DropImpulseZ = 600.f;

private:
	// Alvo atualmente levantado
	TWeakObjectPtr<class ACharacter> LiftedTarget;
	// Controller do inimigo durante a levitação
	TWeakObjectPtr<class AAIController> LiftedAIController;
	bool bAIStopped = false;

	// Z desejado ao término da subida
	float DesiredLiftZ = 0.f;

	FTimerHandle RiseTimerHandle;
	FTimerHandle HangTimerHandle;
	FTimerHandle DamageTimerHandle;
	FTimerHandle RisePollTimerHandle;

	// Intervalo do polling de subida para parar exatamente na altura
	UPROPERTY(EditDefaultsOnly, Category = "Lift")
	float RisePollInterval = 0.02f;

	void OnRiseFinished();
	void OnHangFinished();
	void OnDamageTime();
	void OnRisePoll();
	void CleanupTargetState();

	// Evita disparar o evento de lock no ar mais de uma vez
	bool bHasLockedInAir = false;
};


