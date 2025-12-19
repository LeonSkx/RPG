#include "Quest/QuestDataAsset.h"

FQuestData UQuestDataAsset::FindQuest(const FString& QuestID)
{
    // Buscar apenas em Main Story
    for (const FQuestData& Quest : MainStoryQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    
    return FQuestData();
}

TArray<FQuestData> UQuestDataAsset::GetQuestsByType(EQuestType QuestType)
{
    TArray<FQuestData> Result;
    
    // Por enquanto, só retorna Main Story
    if (QuestType == EQuestType::Main)
    {
        Result = MainStoryQuests;
    }
    
    return Result;
}

TArray<FQuestData> UQuestDataAsset::GetAvailableQuests(int32 CharacterLevel)
{
    TArray<FQuestData> Result;
    
    // Verificar apenas Main Story
    for (const FQuestData& Quest : MainStoryQuests)
    {
        if (Quest.Prerequisites.RequiredLevel <= CharacterLevel)
        {
            Result.Add(Quest);
        }
    }
    
    return Result;
}

TArray<FQuestData> UQuestDataAsset::GetQuestsThatRequire(const FString& RequiredQuestID)
{
    TArray<FQuestData> Result;
    
    // Verificar apenas Main Story
    for (const FQuestData& Quest : MainStoryQuests)
    {
        if (Quest.Prerequisites.RequiredQuests.Contains(RequiredQuestID))
        {
            Result.Add(Quest);
        }
    }
    
    return Result;
}

int32 UQuestDataAsset::GetTotalQuestCount() const
{
    return MainStoryQuests.Num();
}

int32 UQuestDataAsset::GetQuestCountByType(EQuestType QuestType) const
{
    if (QuestType == EQuestType::Main)
    {
        return MainStoryQuests.Num();
    }
    return 0;
}

const TArray<FQuestData>& UQuestDataAsset::GetQuestArrayByType(EQuestType QuestType) const
{
    return MainStoryQuests;
} 