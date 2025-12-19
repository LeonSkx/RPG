#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Quest/QuestTypes.h"
#include "QuestDataAsset.generated.h"

/**
 * Data Asset para armazenar todas as quests do jogo
 */
UCLASS(BlueprintType)
class RPG_API UQuestDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // === QUESTS ORGANIZADAS POR TIPO ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Main Story")
    TArray<FQuestData> MainStoryQuests;

    // === FUNÇÕES DE BUSCA ===

    /** Encontrar quest por ID */
    UFUNCTION(BlueprintCallable, Category = "Quest Data")
    FQuestData FindQuest(const FString& QuestID);

    /** Obter todas as quests de um tipo específico */
    UFUNCTION(BlueprintCallable, Category = "Quest Data")
    TArray<FQuestData> GetQuestsByType(EQuestType QuestType);

    /** Obter todas as quests disponíveis para um nível */
    UFUNCTION(BlueprintCallable, Category = "Quest Data")
    TArray<FQuestData> GetAvailableQuests(int32 CharacterLevel);

    /** Obter quests que requerem uma quest específica */
    UFUNCTION(BlueprintCallable, Category = "Quest Data")
    TArray<FQuestData> GetQuestsThatRequire(const FString& RequiredQuestID);

    /** Obter total de quests */
    UFUNCTION(BlueprintPure, Category = "Quest Data")
    int32 GetTotalQuestCount() const;

    /** Obter total de quests por tipo */
    UFUNCTION(BlueprintPure, Category = "Quest Data")
    int32 GetQuestCountByType(EQuestType QuestType) const;

private:
    /** Obter array de quests baseado no tipo */
    const TArray<FQuestData>& GetQuestArrayByType(EQuestType QuestType) const;
}; 