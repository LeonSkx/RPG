#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Quest/QuestTypes.h"
#include "Quest/QuestDataAsset.h"
#include "Inventory/Core/InventoryTypes.h"
#include "QuestSubsystem.generated.h"

class ARPGCharacter;

// === DELEGATES ===

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAccepted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnObjectiveUpdated, const FString&, QuestID, const FString&, ObjectiveID, int32, Progress);

/**
 * Subsystem para gerenciar sistema de quests
 */
UCLASS()
class RPG_API UQuestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuestSubsystem();

    // === INICIALIZAÇÃO ===

    /** Configurar os Data Assets de quests */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SetQuestDataAssets(const TArray<UQuestDataAsset*>& InQuestDataAssets);

    /** Carregar quests do Data Asset */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void LoadQuestsFromDataAsset();

    // === GESTÃO DE QUESTS ===

    /** Aceitar uma quest */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AcceptQuest(const FString& QuestID, ARPGCharacter* Character);

    /** Completar uma quest */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID, ARPGCharacter* Character);

    /** Abandonar uma quest */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(const FString& QuestID, ARPGCharacter* Character);

    /** Falhar uma quest */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID, ARPGCharacter* Character);

    // === ATUALIZAÇÃO DE PROGRESSO ===

    /** Atualizar progresso de objetivo */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress, ARPGCharacter* Character);

    /** Atualizar progresso de kill */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateKillProgress(const FString& QuestID, const FString& EnemyType, int32 Amount, ARPGCharacter* Character);

    /** Atualizar progresso de kill automaticamente em todas as quests ativas */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateKillProgressAuto(const FString& EnemyType, int32 Amount, ARPGCharacter* Character);

    /** Atualizar progresso de coleta */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateCollectProgress(const FString& QuestID, const FString& ItemID, int32 Amount, ARPGCharacter* Character);

    /** Atualizar progresso de coleta automaticamente em todas as quests ativas */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateCollectProgressAuto(const FString& ItemID, int32 Amount, ARPGCharacter* Character);

    // === ESCUTA DE EVENTOS ===

    /** Escutar mudanças no inventário */
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnInventoryItemChanged(EItemCategory Category, const FInventoryItem& Item);

    // === CONSULTA DE DADOS - QUESTS ===

    /** Verificar se quest está ativa */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID, ARPGCharacter* Character) const;

    /** Verificar se quest está completada */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID, ARPGCharacter* Character) const;

    /** Obter progresso de uma quest */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    FQuestProgress GetQuestProgress(const FString& QuestID, ARPGCharacter* Character) const;

    // ✅ NOVAS FUNÇÕES: Verificação de objetivos
    /** Verificar se objetivo está disponível */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    bool IsObjectiveAvailable(const FString& QuestID, const FString& ObjectiveID, ARPGCharacter* Character) const;

    /** Verificar se objetivo está completado */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    bool IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID, ARPGCharacter* Character) const;

    /** Obter progresso de um objetivo específico */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    int32 GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, ARPGCharacter* Character) const;

    /** Obter todas as quests ativas */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    TArray<FQuestProgress> GetActiveQuests(ARPGCharacter* Character) const;

    /** Obter todas as quests completadas */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    TArray<FQuestProgress> GetCompletedQuests(ARPGCharacter* Character) const;

    /** Obter quests disponíveis para um personagem */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    TArray<FQuestData> GetAvailableQuests(ARPGCharacter* Character) const;

    /** Obter o nome (FText) de uma quest a partir do QuestID (vazio se não encontrada) */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    FText GetQuestNameByID(const FString& QuestID) const;

    /** Obter o texto do tipo de quest a partir do QuestID (vazio se não encontrada) */
    UFUNCTION(BlueprintPure, Category = "Quest System")
    FText GetQuestTypeTextByID(const FString& QuestID) const;

    // === DEBUG - QUESTS ===

    /** Mostrar informações de debug de uma quest */
    UFUNCTION(BlueprintCallable, Category = "Quest System|Debug")
    void DebugQuestInfo(const FString& QuestID, ARPGCharacter* Character);

    /** Mostrar todas as quests ativas */
    UFUNCTION(BlueprintCallable, Category = "Quest System|Debug")
    void DebugShowAllActiveQuests(ARPGCharacter* Character);

    /** Testar aceitar quest */
    UFUNCTION(BlueprintCallable, Category = "Quest System|Debug")
    void DebugAcceptQuest(const FString& QuestID, ARPGCharacter* Character);

    /** Verificar se Data Asset está configurado */
    UFUNCTION(BlueprintCallable, Category = "Quest System|Debug")
    void DebugQuestDataAsset();

    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "Quest System|Events")
    FOnQuestAccepted OnQuestAccepted;

    UPROPERTY(BlueprintAssignable, Category = "Quest System|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest System|Events")
    FOnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest System|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

private:
    // === DADOS - QUESTS ===

    /** Array de Data Assets com quests organizadas por capítulo */
    UPROPERTY()
    TArray<UQuestDataAsset*> QuestDataAssets;

    /** Progresso das quests por personagem */
    UPROPERTY()
    TMap<FString, FQuestProgress> QuestProgressMap;

    // === CACHE DE QUESTDATA ===

    /** Cache para lookup rápido de quests */
    UPROPERTY()
    TMap<FString, FQuestData> QuestDataCache;

    /** Flag para indicar se cache foi construído */
    bool bQuestCacheBuilt = false;

    /** Construir cache de quests */
    void BuildQuestCache();

    /** Buscar quest no cache */
    const FQuestData* FindQuestInCache(const FString& QuestID) const;

    /** Limpar cache */
    void ClearQuestCache();

    // === FUNÇÕES INTERNAS - QUESTS ===

    /** Verificar se personagem pode aceitar quest */
    bool CanAcceptQuest(const FQuestData& QuestData, ARPGCharacter* Character) const;

    /** Verificar se quest pode ser completada */
    bool CanCompleteQuest(const FQuestProgress& Progress) const;

    /** Atualizar estado de uma quest */
    void UpdateQuestState(const FString& QuestID, EQuestState NewState, ARPGCharacter* Character);

    /** Verificar se objetivo foi completado */
    bool IsObjectiveCompleted(const FQuestObjective& Objective) const;

    /** Obter progresso de quest (cria se não existir) */
    FQuestProgress* GetOrCreateQuestProgress(const FString& QuestID, ARPGCharacter* Character);

    /** Obter progresso de quest (apenas leitura) */
    const FQuestProgress* GetQuestProgressInternal(const FString& QuestID, ARPGCharacter* Character) const;

    // === FUNÇÕES INTERNAS - REQUISITOS ===

    /** Verificar requisitos de nível */
    bool CheckLevelRequirement(const FQuestPrerequisites& Prerequisites, ARPGCharacter* Character) const;

    /** Verificar nível médio da party */
    bool CheckPartyAverageLevel(int32 RequiredLevel, ARPGCharacter* Character) const;

    /** Verificar nível de um personagem específico */
    bool CheckCharacterLevel(int32 RequiredLevel, ARPGCharacter* Character) const;

    /** Verificar quests pré-requisitas */
    bool CheckQuestRequirements(const TArray<FString>& RequiredQuests, ARPGCharacter* Character) const;

    /** Verificar itens pré-requisitos */
    bool CheckItemRequirements(const TArray<FString>& RequiredItems, ARPGCharacter* Character) const;

    // === FUNÇÕES INTERNAS - KEY GENERATION ===

    /** Gerar chave única para progresso de quest */
    FString GenerateQuestKey(ARPGCharacter* Character, const FString& QuestID) const;
    
    // === FUNÇÕES INTERNAS - VALIDAÇÕES ===

    /** Validar transições de estado de quest */
    bool CanTransitionQuestState(const FString& QuestID, EQuestState FromState, EQuestState ToState, ARPGCharacter* Character) const;

    // ✅ NOVA FUNÇÃO: Atualizar disponibilidade dos objetivos
    void UpdateObjectiveAvailability(FQuestProgress* QuestProgress);
};