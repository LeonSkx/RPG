// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Base/RPGDamageGameplayAbility.h"
#include "TimerManager.h"
#include "RPGRainOfLightAbility.generated.h"

class ARPGEnemy;
class AAIController;
class AActor;
class UNiagaraSystem;

/**
 * Habilidade Rain of Light
 * Versão funcional: aplica stun em um alvo (central) e dispara evento com localização no chão
 */
UCLASS()
class RPG_API URPGRainOfLightAbility : public URPGDamageGameplayAbility
{
	GENERATED_BODY()

public:
	URPGRainOfLightAbility();

	/**
	 * Função exposta para Blueprint que recebe um alvo e aplica stun
	 */
	UFUNCTION(BlueprintCallable, Category = "Rain of Light")
	void ExecuteRainOfLight(AActor* Target);

	/** Evento disparado quando o alvo entra em stun (envia o alvo e sua localização no chão) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRainTargetStunnedSignature, AActor*, TargetActor, FVector, TargetLocation);

	/** Assinável em BP para reagir ao stun do alvo */
	UPROPERTY(BlueprintAssignable, Category = "Rain of Light|Events")
	FRainTargetStunnedSignature OnTargetStunned;

protected:
	/** Duração do stun em segundos */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rain of Light|Stun")
	float StunDuration = 3.0f;

	/** Raio usado para debug de detecção (SphereOverlap) ao executar a habilidade */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rain of Light|Debug")
	float DetectionRadius = 500.f;

	/** Spawn opcional de Niagara no chão (apenas no alvo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rain of Light|FX")
	bool bSpawnGroundNiagara = false;

	/** Sistema Niagara opcional para spawnar no chão (apenas no alvo) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rain of Light|FX")
	UNiagaraSystem* GroundNiagaraSystem = nullptr;

	/** Escala do sistema Niagara */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rain of Light|FX")
	FVector GroundNiagaraScale = FVector(1.f, 1.f, 1.f);

	/** Aplica stun completo no inimigo especificado (alvo central) */
	void ApplyStunToEnemy(ARPGEnemy* Enemy);

	/** Aplica stun simples (sem efeitos) em inimigos satélites */
	void ApplySatelliteStun(ARPGEnemy* Enemy);

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
						  const FGameplayAbilityActorInfo* ActorInfo,
						  const FGameplayAbilityActivationInfo ActivationInfo,
						  bool bReplicateEndAbility,
						  bool bWasCancelled) override;

	/** Obtém a posição do chão sob o ator. Retorna false se não encontrar. */
	bool GetGroundLocationUnderActor(const AActor* Actor, FVector& OutLocation, FVector& OutNormal) const;

private:
	/** Timer para remover o stun do alvo central */
	FTimerHandle StunTimerHandle;

	/** Referência fraca ao inimigo atualmente stunnado (alvo central) */
	TWeakObjectPtr<ARPGEnemy> StunnedEnemy;

	/** Referência fraca ao AI Controller do alvo central */
	TWeakObjectPtr<AAIController> EnemyAIController;

	/** Timers de stuns dos satélites (sem efeitos) */
	TMap<TWeakObjectPtr<ARPGEnemy>, FTimerHandle> SatelliteStunTimers;

	/** Função para remover o stun do alvo central */
	void RemoveStun();

	/** Remover stun de um satélite específico */
	void RemoveSatelliteStun(ARPGEnemy* Enemy);

	/** Remover todos os stuns de satélites */
	void RemoveAllSatelliteStuns();
};
