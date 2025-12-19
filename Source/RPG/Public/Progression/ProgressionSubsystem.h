// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ProgressionSubsystem.generated.h"

class ARPGCharacter;

// Delegate para level up
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterLevelUp, ARPGCharacter*, Character, int32, NewLevel);

// Delegate para mudança de XP
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCharacterXPChanged, ARPGCharacter*, Character, int32, CurrentXP, int32, XPForNextLevel);

// Delegate para quando o ProgressionSubsystem estiver pronto
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProgressionReady);

USTRUCT(BlueprintType)
struct FCharacterProgressionData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 PlayerLevel = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 XP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 AttributePoints = 50;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 SpellPoints = 50;

	// Construtor padrão
	FCharacterProgressionData()
		: PlayerLevel(1)
		, XP(0)
		, AttributePoints(50)
		, SpellPoints(50)
	{
	}
};

/**
 * Subsistema para gerenciar a progressão de todos os personagens do jogador.
 * Controla Nível, XP, Pontos de Atributo, etc.
 * FONTE DA VERDADE para todos os dados de progressão.
 */
UCLASS()
class RPG_API UProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// === GETTERS ===
	
	// Obtém o nível de um personagem.
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetCharacterLevel(const ARPGCharacter* Character) const;

	// Obtém o XP de um personagem.
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetCharacterXP(const ARPGCharacter* Character) const;

	// Obtém os pontos de atributo de um personagem.
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetCharacterAttributePoints(const ARPGCharacter* Character) const;

	// Obtém os pontos de magia de um personagem.
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetCharacterSpellPoints(const ARPGCharacter* Character) const;

	// === SETTERS ===
	
	// Define o nível de um personagem.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void SetCharacterLevel(ARPGCharacter* Character, int32 NewLevel);

	// Adiciona XP a um personagem.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddCharacterXP(ARPGCharacter* Character, int32 XPToAdd);

	// Define o XP de um personagem.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void SetCharacterXP(ARPGCharacter* Character, int32 NewXP);

	// Adiciona pontos de atributo a um personagem.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddCharacterAttributePoints(ARPGCharacter* Character, int32 PointsToAdd);

	// Define os pontos de atributo de um personagem.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void SetCharacterAttributePoints(ARPGCharacter* Character, int32 NewPoints);

	// Adiciona pontos de magia a um personagem.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddCharacterSpellPoints(ARPGCharacter* Character, int32 PointsToAdd);

	// Define os pontos de magia de um personagem.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void SetCharacterSpellPoints(ARPGCharacter* Character, int32 NewPoints);

	// === FUNÇÕES DE GRUPO ===
	
	// Adiciona XP para todos os personagens do grupo
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddGroupXP(int32 XPToAdd);

	// Adiciona pontos de atributo para todos os personagens do grupo
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddGroupAttributePoints(int32 PointsToAdd);

	// Adiciona pontos de magia para todos os personagens do grupo
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddGroupSpellPoints(int32 PointsToAdd);

	// Adiciona XP para todos os personagens do grupo via GAS
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void AddGroupXPViaGAS(int32 XPToAdd);

	// === CÁLCULOS DE PROGRESSÃO ===
	
	// Calcula o nível baseado no XP usando multiplicadores dinâmicos
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 CalculateLevelFromXP(int32 XP) const;

	// Calcula o XP necessário para um nível específico
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 CalculateXPForLevel(int32 Level) const;

	// Calcula o XP necessário para o próximo nível
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 CalculateXPForNextLevel(const ARPGCharacter* Character) const;

	// Calcula o multiplicador de XP para um nível específico
	UFUNCTION(BlueprintPure, Category = "Progression")
	float CalculateMultiplierForLevel(int32 Level) const;

	// Verifica se o personagem pode subir de nível
	UFUNCTION(BlueprintPure, Category = "Progression")
	bool CanLevelUp(const ARPGCharacter* Character) const;

	// Executa o level up do personagem
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void LevelUpCharacter(ARPGCharacter* Character);

	// Calcula quantos pontos de atributo o personagem ganha ao subir de nível
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetAttributePointsReward(int32 Level) const;

	// Calcula quantos pontos de magia o personagem ganha ao subir de nível
	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetSpellPointsReward(int32 Level) const;

	// Garante que uma entrada de dados de progressão exista para o personagem.
	UFUNCTION(BlueprintCallable, Category = "Progression")
	void EnsureCharacterDataExists(ARPGCharacter* Character);

	// Evento disparado quando um character sobe de nível
	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnCharacterLevelUp OnCharacterLevelUp;

	// Evento disparado quando o XP de um character muda
	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnCharacterXPChanged OnCharacterXPChanged;

	// Evento disparado quando o ProgressionSubsystem estiver pronto
	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnProgressionReady OnProgressionReady;

private:
	
	// XP base para o nível 1
	const int32 BASE_XP = 100;
	
	// Pontos de atributo base por nível
	const int32 BASE_ATTRIBUTE_POINTS = 1;
	
	// Pontos de magia base por nível
	const int32 BASE_SPELL_POINTS = 1;
	
	UPROPERTY()
	TMap<FName, FCharacterProgressionData> ProgressionDataMap;
}; 