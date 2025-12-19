// Copyright (c) 2025 RPG Yumi Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"
#include "Base/RPGDamageGameplayAbility.h"
#include "GA_Combo.generated.h"

/**
 * Habilidade de combo que usa uma única montagem com múltiplas seções.
 * Sistema baseado em eventos de gameplay para mudança de seções e input do jogador.
 * A aplicação de dano deve ser feita pelo sistema de projéteis ou outras habilidades.
 */
UCLASS()
class RPG_API UGA_Combo : public URPGDamageGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Combo();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	// Override InputPressed para escutar o mesmo input que ativou a habilidade
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	
	// Eventos para BP separados por origem (Player vs AI)
	UFUNCTION(BlueprintImplementableEvent, Category = "Combo|Events")
	void OnComboStepStarted_Player();

	UFUNCTION(BlueprintImplementableEvent, Category = "Combo|Events")
	void OnComboStepStarted_AI();
	
	// Retorna o nome do socket em uso no ARPGCharacter (via Equipment/ItemData)
	UFUNCTION(BlueprintPure, Category = "Combo|Damage")
	FName GetCurrentWeaponSocketName() const;

	// Retorna a localização (world) do socket em uso no ARPGCharacter
	UFUNCTION(BlueprintPure, Category = "Combo|Damage")
	FVector GetCurrentWeaponSocketLocation() const;

	// Nomes configurados no EquipmentComponent (principal + alternativos) para o slot Weapon
	UFUNCTION(BlueprintPure, Category = "Combo|Damage")
	TArray<FName> GetConfiguredWeaponSocketNames() const;

	// Nomes de sockets EXISTENTES (que realmente existem no mesh da arma/equip/character), na ordem de prioridade
	UFUNCTION(BlueprintPure, Category = "Combo|Damage")
	TArray<FName> GetExistingWeaponSocketNames() const;

	// Dado um nome de socket, retorna a localização (world) resolvendo arma -> equipado -> personagem
	UFUNCTION(BlueprintPure, Category = "Combo|Damage")
	FVector GetWeaponSocketLocationByName(FName SocketName) const;

	// Bind opcional: quando uma arma for equipada, emite o socket atual
	UFUNCTION(BlueprintImplementableEvent, Category = "Combo|Events")
	void OnWeaponSocketUpdated(FName SocketName, FVector SocketLocation);

	// === SISTEMA UNIFICADO: COMBO + PROJÉTIL ===
	
	// Spawna projétil durante o combo (chamado por notifies)
	// PlayerLocation: localização do mouse/crosshair (Player)
	// ActorLocation: localização do alvo de combate (IA)
	// bUseManualSpawnOrigin: se verdadeiro, usa ManualSpawnOrigin em vez do socket da arma
	// ManualSpawnOrigin: localização manual da origem do spawn (usado se bUseManualSpawnOrigin = true)
	UFUNCTION(BlueprintCallable, Category = "Combo|Projectile")
	void SpawnProjectile(const FVector& PlayerLocation, const FVector& ActorLocation, 
	                     bool bUseManualSpawnOrigin = false, const FVector& ManualSpawnOrigin = FVector::ZeroVector);

	// Função com a mesma entrada do SpawnProjectile, renomeada
	UFUNCTION(BlueprintCallable, Category = "Combo|Damage")
	void DamageLocation(const FVector& ProjectileTargetLocation);
	
	static FGameplayTag GetComboChangedEventTag();
	static FGameplayTag GetComboChangedEventEndTag();

protected:
	// === SISTEMA UNIFICADO: PROJÉTEIS ===
	
	// Se verdadeiro, spawna projétil em vez de aplicar dano direto
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combo|Projectile")
	bool bUseProjectiles = false;

	// Classe do projétil a ser spawnado (se bUseProjectiles = true)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo|Projectile",
	          meta = (EditCondition = "bUseProjectiles"))
	TSubclassOf<class ARPGProjectile> ProjectileClass;

private:
	void TryCommitCombo();

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* ComboMontage;

	UFUNCTION()
	void ComboChangedEventReceived(FGameplayEventData Data);

	FName NextComboName;
	bool bComboWindowOpen;
};