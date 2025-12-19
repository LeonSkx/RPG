// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Quest/QuestDataAsset.h"
#include "RPGGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API URPGGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	UPROPERTY()
	FName PlayerStartTag = FName();

	UPROPERTY()
	FString LoadSlotName = FString();

	UPROPERTY()
	int32 LoadSlotIndex = 0;

    // Sistema de save removido do projeto

	// === QUEST SYSTEM ===

	/** Array de Data Assets com quests organizadas por capítulo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
	TArray<UQuestDataAsset*> QuestDataAssets;

protected:
	/** Inicialização do Game Instance */
	virtual void Init() override;
};
