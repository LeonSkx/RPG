// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Base/RPGDamageGameplayAbility.h"
#include "RPGTeleportAbility.generated.h"

/**
 * Habilidade de teleporte direcional
 * Permite ao personagem se teleportar na direção do input
 */
UCLASS()
class RPG_API URPGTeleportAbility : public URPGDamageGameplayAbility
{
	GENERATED_BODY()

public:
	URPGTeleportAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
								const FGameplayAbilityActorInfo* ActorInfo,
								const FGameplayAbilityActivationInfo ActivationInfo,
								const FGameplayEventData* TriggerEventData) override;

protected:
	/**
	 * Executa o teleporte na direção especificada
	 */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	bool ExecuteTeleport(const FVector& Direction);

	/**
	 * Calcula a posição de destino do teleporte
	 */
	UFUNCTION(BlueprintPure, Category = "Teleport")
	FVector CalculateTeleportDestination(const FVector& Direction) const;

	/**
	 * Obtém a direção do input do jogador
	 */
	UFUNCTION(BlueprintPure, Category = "Teleport")
	FVector GetInputDirection() const;

protected:
	/** Distância do teleporte em unidades */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float TeleportDistance = 300.f;

	/** Altura máxima permitida para teleporte */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float MaxTeleportHeight = 100.f;

	/** Raio de verificação de colisão */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float CollisionCheckRadius = 50.f;

	/** Distância mínima para teleporte */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float MinTeleportDistance = 100.f;

	/** Distância para verificar chão */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float GroundCheckDistance = 200.f;

	/** Número máximo de tentativas de teleporte */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	int32 MaxTeleportAttempts = 3;

	/** Se deve verificar se há chão no destino */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	bool bRequireGroundAtDestination = true;

	/** Se deve usar direção do input de movimento */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	bool bUseMovementInputDirection = true;

	/** Se deve usar direção da câmera (controller) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	bool bUseCameraDirection = false;

	/** Se deve exigir que o personagem esteja se movendo para teleportar */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	bool bRequireMovement = true;

	/** Velocidade mínima para considerar que o personagem está se movendo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float MovementThreshold = 10.f;

protected:
	/**
	 * Tenta direções alternativas se o teleporte original falhar
	 */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	bool TryAlternativeDirections(const FVector& OriginalDirection);

	/**
	 * Encontra uma posição válida para teleporte
	 */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	bool FindValidTeleportDestination(const FVector& DesiredDestination, FVector& OutValidDestination) const;

	/**
	 * Verifica se uma posição está livre de colisões
	 */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	bool IsLocationClear(const FVector& Location) const;

	/**
	 * Encontra o chão em uma posição específica
	 */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	bool FindGroundAtLocation(const FVector& Location, FVector& OutGroundLocation) const;

	/**
	 * Chamado quando o teleporte é bem-sucedido
	 */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	void OnTeleportSuccess(const FVector& OldLocation, const FVector& NewLocation);

	/**
	 * Chamado quando o teleporte falha
	 */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	void OnTeleportFailed();

    // Nova função para chamar no Blueprint quando quiser
    UFUNCTION(BlueprintCallable, Category = "Teleport")
    bool TryTeleport(const FVector& Direction);

    // Função para ativar o teleport quando os efeitos estiverem prontos
    UFUNCTION(BlueprintCallable, Category = "Teleport")
    void ActivateTeleport();
}; 