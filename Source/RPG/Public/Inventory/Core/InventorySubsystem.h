#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Inventory/Core/InventoryTypes.h" // Para FInventoryItem, EInventoryActionResult
#include "Inventory/Core/InventoryEnums.h" // Para EInventoryFilterCategory
#include "InventorySubsystem.generated.h"

class UItemDataAsset;
// Delegate FOnInventoryChanged já é declarado em InventoryTypes.h (TwoParams)

/**
 * Subsistema para gerenciar um inventário compartilhado pela party,
 * focado em consumíveis e itens gerais.
 */
UCLASS()
class RPG_API UInventorySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ----- Inicialização -----
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ----- Operações de Item -----

    /** Adiciona um item (por DataAsset) ao inventário. Tenta empilhar se possível. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    EInventoryActionResult AddItem(UItemDataAsset* ItemToAdd, int32 Quantity = 1, int32 Level = 1);

    /** Adiciona um FInventoryItem já existente (ex: transferido). Tenta empilhar. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    EInventoryActionResult AddInventoryItem(const FInventoryItem& Item);

    /** Remove uma quantidade de um item (por DataAsset). */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    EInventoryActionResult RemoveItem(UItemDataAsset* ItemToRemove, int32 Quantity = 1, int32 Level = 1);

    /** Remove uma quantidade de um item específico (por ID) - útil para itens únicos transferidos. */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    EInventoryActionResult RemoveItemByID(const FGuid& ItemID, int32 Quantity = 1);

    // --- Funções de Transferência ---

    // Removido: transferência para inventário de personagem
    
    // --- Fim Funções de Transferência ---

    // ----- Consultas -----

    /** Verifica se o inventário contém uma quantidade de um item. */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasItem(UItemDataAsset* Item, int32 Quantity = 1, int32 Level = 1) const;

    /** Retorna a quantidade total de um item específico no inventário. */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetItemCount(UItemDataAsset* Item, int32 Level = 1) const;

    /** Retorna uma cópia de todos os itens no inventário. */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventoryItem> GetAllItems() const { return SharedItems; }

    /** Retorna itens filtrados por classe específica. */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventoryItem> GetItemsForClass(EPlayerClass PlayerClass) const;

    /** Marca um item (por ID) como visto, removendo o status de "Novo". */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void MarkItemAsSeen(const FGuid& ItemID);

    /** Retorna itens filtrados por categoria de UI: Novo, Consumível, Arma, Acessório. */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FInventoryItem> GetItemsByFilter(EInventoryFilterCategory Filter) const;

    /** Retorna um item pelo seu ID único (se existir no inventário). */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    FInventoryItem GetItemByID(const FGuid& ItemID) const;
    
    // ----- Capacidade (Exemplo Simples) -----

    /** Define a capacidade máxima de slots no inventário (-1 para ilimitado). */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetInventoryCapacity(int32 NewCapacity) { MaxCapacity = NewCapacity; }

    /** Retorna a capacidade máxima de slots. */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetInventoryCapacity() const { return MaxCapacity; }

    /** Retorna o número de slots ocupados. */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetUsedSlotsCount() const { return SharedItems.Num(); }

    /** Verifica se há espaço para adicionar um novo tipo de item (um novo slot). */
    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasFreeSlot() const { return MaxCapacity < 0 || SharedItems.Num() < MaxCapacity; }

    // ----- Sistema de Gold -----

    /** Adiciona gold ao inventário. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Gold")
    void AddGold(int32 Amount);

    /** Remove gold do inventário. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Gold")
    bool RemoveGold(int32 Amount);

    /** Retorna a quantidade atual de gold. */
    UFUNCTION(BlueprintPure, Category = "Inventory|Gold")
    int32 GetGold() const { return CurrentGold; }

    /** Verifica se há gold suficiente. */
    UFUNCTION(BlueprintPure, Category = "Inventory|Gold")
    bool HasGold(int32 Amount) const { return CurrentGold >= Amount; }

    /** Define a quantidade de gold (útil para debug/save). */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Gold")
    void SetGold(int32 NewAmount);

    // ----- Eventos -----

    /** Delegate disparado quando um item é adicionado, removido ou sua quantidade muda. */
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    /** Delegate disparado quando o gold muda. */
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnGoldChanged OnGoldChanged;

protected:
    /** Lista dos itens no inventário. */
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TArray<FInventoryItem> SharedItems;

    /** Quantidade atual de gold no inventário. */
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    int32 CurrentGold = 0;

    /** Capacidade máxima de slots (-1 = ilimitado). */
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    int32 MaxCapacity = 100;

    // === OTIMIZAÇÕES DE PERFORMANCE ===
    
    /** 
     * Cache de índices por ID para busca O(1) 
     * Mapa: ItemID -> Índice no array SharedItems
     */
    UPROPERTY(Transient)
    TMap<FGuid, int32> ItemIndexCache;
    
    /** 
     * Cache de contadores por tipo de item para evitar recálculos
     * Mapa: (ItemData + Level) -> Quantidade total
     */
    UPROPERTY(Transient)
    TMap<FString, int32> ItemCountCache;
    
    /** 
     * Flag para indicar se os caches estão válidos
     * Invalidado quando há mudanças no inventário
     */
    UPROPERTY(Transient)
    bool bCacheValid = false;
    
    /** 
     * Batch de mudanças pendentes para evitar múltiplos broadcasts
     * Acumula mudanças e faz broadcast único no final do frame
     */
    UPROPERTY(Transient)
    TArray<FInventoryItem> PendingChanges;
    
    /** Timer handle para batch de broadcasts */
    FTimerHandle BatchBroadcastTimer;

private:
    /** Encontra o índice de um item empilhável existente que corresponda (DataAsset e Level). */
    int32 FindStackableItemIndex(UItemDataAsset* ItemData, int32 Level) const;

    /** Encontra o índice de um item pelo seu UniqueID. */
    int32 FindItemIndexByID(const FGuid& ItemID) const;

    /** Notifica que um item mudou (para disparar o delegate). */
    void BroadcastInventoryChange(const FInventoryItem& Item);
    
    // === FUNÇÕES DE OTIMIZAÇÃO ===
    
    /** Reconstrói todos os caches de performance */
    void RebuildCaches();
    
    /** Invalida os caches (chamado quando inventário muda) */
    void InvalidateCaches();
    
    /** Atualiza cache de um item específico */
    void UpdateItemCache(const FInventoryItem& Item, bool bAdding);
    
    /** Gera chave única para cache de contadores (ItemData + Level) */
    FString GenerateItemCacheKey(UItemDataAsset* ItemData, int32 Level) const;
    
    /** Faz broadcast em batch das mudanças pendentes */
    UFUNCTION()
    void ProcessPendingBroadcasts();
    
    /** Adiciona mudança ao batch (em vez de broadcast imediato) */
    void AddToPendingChanges(const FInventoryItem& Item);

    /** Helpers de filtro */
    bool DoesItemMatchFilter(const FInventoryItem& Item, EInventoryFilterCategory Filter) const;
    
    // === FUNÇÕES DE BENCHMARK/DEBUG ===
    
    /** Testa performance do sistema de inventário com N operações */
    UFUNCTION(BlueprintCallable, Category = "Shared Inventory|Debug", CallInEditor)
    void BenchmarkInventoryPerformance(int32 NumOperations = 1000);
    
    /** Força rebuild dos caches (para testes) */
    UFUNCTION(BlueprintCallable, Category = "Shared Inventory|Debug", CallInEditor)
    void ForceRebuildCaches() { RebuildCaches(); }
    
    /** Obtém estatísticas dos caches */
    UFUNCTION(BlueprintPure, Category = "Shared Inventory|Debug")
    FString GetCacheStats() const;
}; 
