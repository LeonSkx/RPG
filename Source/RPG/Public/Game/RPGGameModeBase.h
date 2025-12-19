// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RPGGameModeBase.generated.h"

class UEnemyClassInfo;
class UPlayerClassInfo;

/**
 * GameMode base do RPG
 */
UCLASS()
class RPG_API ARPGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ARPGGameModeBase();
	
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UEnemyClassInfo> EnemyClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Player Class Defaults")
	TObjectPtr<UPlayerClassInfo> PlayerClassInfo;

	/** Mapa de Data Tables por personagem */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill Trees")
	TMap<FName, UDataTable*> CharacterSkillTables;

protected:
	virtual void BeginPlay() override;

	/** Configura as árvores de habilidades por personagem */
	UFUNCTION(BlueprintCallable, Category="Skill Trees")
	void SetupSkillTrees();
};
