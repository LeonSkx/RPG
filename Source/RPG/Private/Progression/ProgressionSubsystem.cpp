// Copyright Druid Mechanics

#include "Progression/ProgressionSubsystem.h"
#include "Character/RPGCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "RPGGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"

// === GETTERS ===

int32 UProgressionSubsystem::GetCharacterLevel(const ARPGCharacter* Character) const
{
	if (Character)
	{
		FName CharacterID = Character->GetCharacterUniqueID();
		if (const FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			return Data->PlayerLevel;
		}
	}
	// Retorna o nível padrão se o personagem não for encontrado no mapa.
	return FCharacterProgressionData().PlayerLevel;
}

int32 UProgressionSubsystem::GetCharacterXP(const ARPGCharacter* Character) const
{
	if (Character)
	{
		FName CharacterID = Character->GetCharacterUniqueID();
		if (const FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			return Data->XP;
		}
	}
	return 0;
}

int32 UProgressionSubsystem::GetCharacterAttributePoints(const ARPGCharacter* Character) const
{
	if (Character)
	{
		FName CharacterID = Character->GetCharacterUniqueID();
		if (const FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			return Data->AttributePoints;
		}
	}
	return 0;
}

int32 UProgressionSubsystem::GetCharacterSpellPoints(const ARPGCharacter* Character) const
{
	if (Character)
	{
		FName CharacterID = Character->GetCharacterUniqueID();
		if (const FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			return Data->SpellPoints;
		}
	}
	return 0;
}

// === SETTERS ===

void UProgressionSubsystem::SetCharacterLevel(ARPGCharacter* Character, int32 NewLevel)
{
	if (Character)
	{
		EnsureCharacterDataExists(Character);
		FName CharacterID = Character->GetCharacterUniqueID();
		if (FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			Data->PlayerLevel = FMath::Max(1, NewLevel);
		}
	}
}

void UProgressionSubsystem::AddCharacterXP(ARPGCharacter* Character, int32 XPToAdd)
{
	if (Character && XPToAdd > 0)
	{
		EnsureCharacterDataExists(Character);
		FName CharacterID = Character->GetCharacterUniqueID();
		if (FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			int32 OldXP = Data->XP;
			int32 OldLevel = Data->PlayerLevel;
			
			Data->XP += XPToAdd;
			

			
			// Verificar se pode subir de nível
			int32 LevelUps = 0;
			while (CanLevelUp(Character))
			{
				LevelUpCharacter(Character);
				LevelUps++;
			}
			
			// Disparar evento de mudança de XP DEPOIS do level up
			int32 XPForNextLevel = CalculateXPForNextLevel(Character);
			OnCharacterXPChanged.Broadcast(Character, Data->XP, XPForNextLevel);
			

		}
	}
}

void UProgressionSubsystem::SetCharacterXP(ARPGCharacter* Character, int32 NewXP)
{
	if (Character)
	{
		EnsureCharacterDataExists(Character);
		FName CharacterID = Character->GetCharacterUniqueID();
		if (FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			Data->XP = FMath::Max(0, NewXP);
			
			// Verificar se pode subir de nível
			int32 LevelUps = 0;
			while (CanLevelUp(Character))
			{
				LevelUpCharacter(Character);
				LevelUps++;
			}
			
			// Disparar evento de mudança de XP DEPOIS do level up
			int32 XPForNextLevel = CalculateXPForNextLevel(Character);
			OnCharacterXPChanged.Broadcast(Character, Data->XP, XPForNextLevel);
		}
	}
}

void UProgressionSubsystem::AddCharacterAttributePoints(ARPGCharacter* Character, int32 PointsToAdd)
{
	if (Character)
	{
		EnsureCharacterDataExists(Character);
		FName CharacterID = Character->GetCharacterUniqueID();
		if (FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			Data->AttributePoints += PointsToAdd;
		}
	}
}

void UProgressionSubsystem::SetCharacterAttributePoints(ARPGCharacter* Character, int32 NewPoints)
{
	if (Character)
	{
		EnsureCharacterDataExists(Character);
		FName CharacterID = Character->GetCharacterUniqueID();
		if (FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			Data->AttributePoints = FMath::Max(0, NewPoints);
		}
	}
}

void UProgressionSubsystem::AddCharacterSpellPoints(ARPGCharacter* Character, int32 PointsToAdd)
{
	if (Character)
	{
		EnsureCharacterDataExists(Character);
		FName CharacterID = Character->GetCharacterUniqueID();
		if (FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			Data->SpellPoints += PointsToAdd;
		}
	}
}

void UProgressionSubsystem::SetCharacterSpellPoints(ARPGCharacter* Character, int32 NewPoints)
{
	if (Character)
	{
		EnsureCharacterDataExists(Character);
		FName CharacterID = Character->GetCharacterUniqueID();
		if (FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
		{
			Data->SpellPoints = FMath::Max(0, NewPoints);
		}
	}
}

// === CÁLCULOS DE PROGRESSÃO ===

int32 UProgressionSubsystem::CalculateLevelFromXP(int32 XP) const
{
	if (XP <= 0) return 1;
	
	// Como agora temos multiplicadores variáveis, usamos uma abordagem iterativa
	// para encontrar o nível correto
	for (int32 Level = 1; Level <= 999; Level++)
	{
		int32 XPForThisLevel = CalculateXPForLevel(Level);
		int32 XPForNextLevel = CalculateXPForLevel(Level + 1);
		
		// Se o XP está entre este nível e o próximo, encontramos o nível correto
		if (XP >= XPForThisLevel && XP < XPForNextLevel)
		{
			return Level;
		}
	}
	
	// Se chegou até aqui, é nível máximo
	return 999;
}

int32 UProgressionSubsystem::CalculateXPForLevel(int32 Level) const
{
	if (Level <= 1) return 0;
	
	// Calcular multiplicador baseado no nível
	float DynamicMultiplier = CalculateMultiplierForLevel(Level);
	
	// Fórmula exponencial com multiplicador dinâmico
	double XPForLevel = BASE_XP * FMath::Pow(DynamicMultiplier, Level - 1);
	
	return FMath::RoundToInt(XPForLevel);
}

float UProgressionSubsystem::CalculateMultiplierForLevel(int32 Level) const
{
	if (Level <= 10)
	{
		// Níveis 1-10: multiplicador 1.3 (fácil)
		return 1.3f;
	}
	else if (Level <= 30)
	{
		// Níveis 11-30: multiplicador aumenta progressivamente de 1.3 para 1.5
		float Progress = static_cast<float>(Level - 10) / 20.0f; // 0.0 a 1.0
		return FMath::Lerp(1.3f, 1.5f, Progress);
	}
	else if (Level <= 50)
	{
		// Níveis 31-50: multiplicador aumenta progressivamente de 1.5 para 1.8
		float Progress = static_cast<float>(Level - 30) / 20.0f; // 0.0 a 1.0
		return FMath::Lerp(1.5f, 1.8f, Progress);
	}
	else if (Level <= 100)
	{
		// Níveis 51-100: multiplicador aumenta progressivamente de 1.8 para 2.2
		float Progress = static_cast<float>(Level - 50) / 50.0f; // 0.0 a 1.0
		return FMath::Lerp(1.8f, 2.2f, Progress);
	}
	else
	{
		// Níveis 101-999: multiplicador fixo em 2.2
		return 2.2f;
	}
}

int32 UProgressionSubsystem::CalculateXPForNextLevel(const ARPGCharacter* Character) const
{
	if (!Character) return BASE_XP;
	
	int32 CurrentLevel = GetCharacterLevel(Character);
	return CalculateXPForLevel(CurrentLevel + 1);
}

bool UProgressionSubsystem::CanLevelUp(const ARPGCharacter* Character) const
{
	if (!Character) return false;
	
	int32 CurrentXP = GetCharacterXP(Character);
	int32 CurrentLevel = GetCharacterLevel(Character);
	int32 XPForNextLevel = CalculateXPForLevel(CurrentLevel + 1);
	
	bool bCanLevelUp = CurrentXP >= XPForNextLevel;
	
	return bCanLevelUp;
}

void UProgressionSubsystem::LevelUpCharacter(ARPGCharacter* Character)
{
	if (!Character) return;
	
	EnsureCharacterDataExists(Character);
	FName CharacterID = Character->GetCharacterUniqueID();
	if (FCharacterProgressionData* Data = ProgressionDataMap.Find(CharacterID))
	{
		int32 OldLevel = Data->PlayerLevel;
		int32 NewLevel = OldLevel + 1;
		
		// Atualizar nível
		Data->PlayerLevel = NewLevel;
		
		// Adicionar pontos de atributo
		int32 AttributePointsReward = GetAttributePointsReward(NewLevel);
		Data->AttributePoints += AttributePointsReward;
		
		// Adicionar pontos de magia
		int32 SpellPointsReward = GetSpellPointsReward(NewLevel);
		Data->SpellPoints += SpellPointsReward;
		
		// Disparar evento de level up
		OnCharacterLevelUp.Broadcast(Character, NewLevel);
		
		// TODO: Atualizar atributos do GAS baseado no novo nível
	}
}

int32 UProgressionSubsystem::GetAttributePointsReward(int32 Level) const
{
	// Base de 1 ponto por nível, pode ser expandido com fórmulas mais complexas
	return BASE_ATTRIBUTE_POINTS;
}

int32 UProgressionSubsystem::GetSpellPointsReward(int32 Level) const
{
	// Base de 1 ponto por nível, pode ser expandido com fórmulas mais complexas
	return BASE_SPELL_POINTS;
}

void UProgressionSubsystem::EnsureCharacterDataExists(ARPGCharacter* Character)
{
	if (Character)
	{
		FName CharacterID = Character->GetCharacterUniqueID();
		if (!ProgressionDataMap.Contains(CharacterID))
		{
			ProgressionDataMap.Add(CharacterID, FCharacterProgressionData());
			
			// Broadcast que o ProgressionSubsystem está pronto para este personagem
			OnProgressionReady.Broadcast();
		}
	}
}

// === FUNÇÕES DE GRUPO ===

void UProgressionSubsystem::AddGroupXP(int32 XPToAdd)
{
	if (XPToAdd <= 0) return;
	
	// Adicionar XP para todos os personagens no mapa
	for (auto& Pair : ProgressionDataMap)
	{
		FCharacterProgressionData& Data = Pair.Value;
		int32 OldXP = Data.XP;
		int32 OldLevel = Data.PlayerLevel;
		
		Data.XP += XPToAdd;
		
		// Verificar se pode subir de nível
		while (Data.XP >= CalculateXPForLevel(Data.PlayerLevel + 1))
		{
			Data.PlayerLevel++;
			
			// Adicionar pontos de atributo
			int32 AttributePointsReward = GetAttributePointsReward(Data.PlayerLevel);
			Data.AttributePoints += AttributePointsReward;
			
			// Adicionar pontos de magia
			int32 SpellPointsReward = GetSpellPointsReward(Data.PlayerLevel);
			Data.SpellPoints += SpellPointsReward;
		}
	}
}

void UProgressionSubsystem::AddGroupAttributePoints(int32 PointsToAdd)
{
	if (PointsToAdd <= 0) return;
	
	// Adicionar pontos de atributo para todos os personagens no mapa
	for (auto& Pair : ProgressionDataMap)
	{
		FCharacterProgressionData& Data = Pair.Value;
		Data.AttributePoints += PointsToAdd;
	}
}

void UProgressionSubsystem::AddGroupSpellPoints(int32 PointsToAdd)
{
	if (PointsToAdd <= 0) return;
	
	// Adicionar pontos de magia para todos os personagens no mapa
	for (auto& Pair : ProgressionDataMap)
	{
		FCharacterProgressionData& Data = Pair.Value;
		Data.SpellPoints += PointsToAdd;
	}
}

void UProgressionSubsystem::AddGroupXPViaGAS(int32 XPToAdd)
{
	if (XPToAdd <= 0) return;
	
	// Buscar todos os personagens no mundo
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> FoundActors;
		TSubclassOf<ARPGCharacter> CharacterClass = ARPGCharacter::StaticClass();
		UGameplayStatics::GetAllActorsOfClass(World, CharacterClass, FoundActors);
		
		for (AActor* Actor : FoundActors)
		{
			if (ARPGCharacter* Character = Cast<ARPGCharacter>(Actor))
			{
				// Verificar se é um personagem do jogador (não inimigo)
				if (Character->Implements<UPlayerInterface>())
				{
					// Usar GAS IncomingXP para cada personagem
					const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
					FGameplayEventData Payload;
					Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;
					Payload.EventMagnitude = XPToAdd;

					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
						Character, 
						GameplayTags.Attributes_Meta_IncomingXP, 
						Payload
					);
				}
			}
		}
	}
} 