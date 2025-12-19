#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Quest/QuestTypes.h"
#include "QuestFunctionLibrary.generated.h"

/**
 * Biblioteca de funções utilitárias para o sistema de Quests
 */
UCLASS()
class RPG_API UQuestFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Retorna o texto (localizável) do tipo de quest usando o DisplayName do enum. */
    UFUNCTION(BlueprintPure, Category = "Quest|Text")
    static FText GetQuestTypeText(EQuestType QuestType);
};

