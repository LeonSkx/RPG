#include "Inventory/Core/InventorySubsystem.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Inventory/Core/InventoryTypes.h"
#include "UObject/NameTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UInventorySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Inicializar caches
    ItemIndexCache.Reserve(100); // Reservar espaço para 100 itens
    ItemCountCache.Reserve(50);  // Reservar espaço para 50 tipos diferentes
    bCacheValid = true;
    
    // Carregar dados salvos aqui, se necessário
}

void UInventorySubsystem::Deinitialize()
{
    // Salvar dados aqui, se necessário
    Super::Deinitialize();
}

EInventoryActionResult UInventorySubsystem::AddItem(UItemDataAsset* ItemToAdd, int32 Quantity, int32 Level)
{
    if (!ItemToAdd || Quantity <= 0)
    {
        return EInventoryActionResult::Failed_InvalidItem;
    }

    FInventoryItem NewItem(ItemToAdd, Quantity, Level);
    return AddInventoryItem(NewItem);
}

EInventoryActionResult UInventorySubsystem::AddInventoryItem(const FInventoryItem& ItemToAdd)
{
    if (!ItemToAdd.IsValid())
    {
        return EInventoryActionResult::Failed_InvalidItem;
    }

    UItemDataAsset* ItemData = ItemToAdd.ItemData;
    int32 RemainingQuantity = ItemToAdd.Quantity;

    // OTIMIZAÇÃO: Rebuild cache se inválido
    if (!bCacheValid)
    {
        RebuildCaches();
    }

    // Tentar empilhar com itens existentes
    if (ItemData->bIsStackable)
    {
        int32 ExistingIndex = FindStackableItemIndex(ItemData, ItemToAdd.Level);
        if (ExistingIndex != INDEX_NONE)
        {
            FInventoryItem& ExistingItem = SharedItems[ExistingIndex];
            int32 CanAdd = ItemData->MaxStackSize - ExistingItem.Quantity;
            int32 AmountToAdd = FMath::Min(RemainingQuantity, CanAdd);

            if (AmountToAdd > 0)
            {
                ExistingItem.Quantity += AmountToAdd;
                ExistingItem.bIsNew = true; // marcar stack existente como novo
                RemainingQuantity -= AmountToAdd;
                
                // OTIMIZAÇÃO: Atualizar cache incrementalmente
                FInventoryItem ChangeNotification = ExistingItem;
                ChangeNotification.Quantity = AmountToAdd;
                UpdateItemCache(ChangeNotification, true);
                
                BroadcastInventoryChange(ChangeNotification);
                UE_LOG(LogTemp, Verbose, TEXT("Added %d to existing stack of %s. Remaining: %d"), AmountToAdd, *ItemData->GetName(), RemainingQuantity);
            }
        }
    }

    // Adicionar como novo slot(s) se ainda houver quantidade restante
    while (RemainingQuantity > 0)
    {
        // Verificar se há espaço
        if (!HasFreeSlot())
        {
            return EInventoryActionResult::Failed_NoSpace;
        }

        // Calcular quanto adicionar neste slot
        int32 AmountThisSlot = ItemData->bIsStackable ? 
            FMath::Min(RemainingQuantity, ItemData->MaxStackSize) : 
            RemainingQuantity;

        // Criar novo item
        FInventoryItem NewSlot(ItemData, AmountThisSlot, ItemToAdd.Level);
        // Marcar como novo para UX (filtro "Novo")
        NewSlot.bIsNew = true;
        // Garantir UniqueID novo ao inserir em novo slot
        NewSlot.UniqueID = FGuid::NewGuid();
        SharedItems.Add(NewSlot);
        
        // OTIMIZAÇÃO: Invalidar cache pois array mudou de tamanho
        InvalidateCaches();
        
        RemainingQuantity -= AmountThisSlot;
        BroadcastInventoryChange(NewSlot);
        
        UE_LOG(LogTemp, Verbose, TEXT("Created new slot for %s with %d items. Remaining: %d"), *ItemData->GetName(), AmountThisSlot, RemainingQuantity);

        // Para itens não empilháveis, sair após adicionar um
        if (!ItemData->bIsStackable)
        {
            break;
        }
    }

    return RemainingQuantity == 0 ? EInventoryActionResult::Success : EInventoryActionResult::Failed_NoSpace;
}

EInventoryActionResult UInventorySubsystem::RemoveItem(UItemDataAsset* ItemToRemove, int32 Quantity, int32 Level)
{
    if (!ItemToRemove || Quantity <= 0)
    {
        return EInventoryActionResult::Failed_InvalidItem;
    }

    int32 RemainingToRemove = Quantity;
    bool bFoundAny = false;

    // Iterar de trás para frente para facilitar remoção
    for (int32 i = SharedItems.Num() - 1; i >= 0 && RemainingToRemove > 0; --i)
    {
        FInventoryItem& CurrentItem = SharedItems[i];
        if (CurrentItem.ItemData == ItemToRemove && CurrentItem.Level == Level)
        {
            bFoundAny = true;
            int32 AmountToRemoveFromSlot = FMath::Min(RemainingToRemove, CurrentItem.Quantity);
            
            CurrentItem.Quantity -= AmountToRemoveFromSlot;
            RemainingToRemove -= AmountToRemoveFromSlot;

            FInventoryItem ChangeNotification = CurrentItem; // Copia antes de potencialmente remover
            ChangeNotification.Quantity = AmountToRemoveFromSlot; // Notifica sobre a quantidade removida

            if (CurrentItem.Quantity <= 0)
            {
                UE_LOG(LogTemp, Verbose, TEXT("Removing slot for %s (Index %d)"), *ItemToRemove->GetName(), i);
                SharedItems.RemoveAt(i);
            }
             else {
                 UE_LOG(LogTemp, Verbose, TEXT("Removed %d from slot %s (Index %d)"), AmountToRemoveFromSlot, *ItemToRemove->GetName(), i);
             }
            
            BroadcastInventoryChange(ChangeNotification); // Notifica sobre a mudança
        }
    }

    if (!bFoundAny)
    {
        return EInventoryActionResult::Failed_NotFound;
    }
    if (RemainingToRemove > 0)
    {
        // Não removemos tudo que foi pedido
        return EInventoryActionResult::Failed_NotEnough; 
    }

    return EInventoryActionResult::Success;
}

EInventoryActionResult UInventorySubsystem::RemoveItemByID(const FGuid& ItemID, int32 Quantity)
{
    if (!ItemID.IsValid() || Quantity <= 0)
    {
        return EInventoryActionResult::Failed_InvalidItem;
    }

    // OTIMIZAÇÃO: Rebuild cache se inválido
    if (!bCacheValid)
    {
        RebuildCaches();
    }

    int32 Index = FindItemIndexByID(ItemID);
    if (Index == INDEX_NONE)
    {
        return EInventoryActionResult::Failed_NotFound;
    }

    FInventoryItem& Item = SharedItems[Index];
    if (Item.Quantity < Quantity)
    {
        return EInventoryActionResult::Failed_NotEnough;
    }

    // Criar notificação antes de modificar
    FInventoryItem ChangeNotification = Item;
    ChangeNotification.Quantity = Quantity;

    Item.Quantity -= Quantity;

    if (Item.Quantity <= 0)
    {
        UE_LOG(LogTemp, Verbose, TEXT("Removing item by ID %s"), *ItemID.ToString());
        SharedItems.RemoveAt(Index);
        
        // OTIMIZAÇÃO: Invalidar cache pois array mudou
        InvalidateCaches();
    }
    else 
    {
        UE_LOG(LogTemp, Verbose, TEXT("Removed %d from item ID %s"), Quantity, *ItemID.ToString());
        
        // OTIMIZAÇÃO: Atualizar cache incrementalmente
        UpdateItemCache(ChangeNotification, false);
    }
    
    BroadcastInventoryChange(ChangeNotification);

    return EInventoryActionResult::Success;
}


bool UInventorySubsystem::HasItem(UItemDataAsset* Item, int32 Quantity, int32 Level) const
{
    return GetItemCount(Item, Level) >= Quantity;
}

int32 UInventorySubsystem::GetItemCount(UItemDataAsset* Item, int32 Level) const
{
    if (!Item) return 0;

    // OTIMIZAÇÃO: Usar cache se válido
    if (bCacheValid)
    {
        FString CacheKey = GenerateItemCacheKey(Item, Level);
        if (const int32* CachedCount = ItemCountCache.Find(CacheKey))
        {
            return *CachedCount;
        }
        // Se não encontrou no cache, significa que não temos esse item
        return 0;
    }

    // Sem fallback: garantir cache válido e consultar
    const_cast<UInventorySubsystem*>(this)->RebuildCaches();
    FString CacheKey = GenerateItemCacheKey(Item, Level);
    if (const int32* CachedCount = ItemCountCache.Find(CacheKey))
    {
        return *CachedCount;
    }
    return 0;
}

FInventoryItem UInventorySubsystem::GetItemByID(const FGuid& ItemID) const
{
    int32 Index = FindItemIndexByID(ItemID);
    if (Index != INDEX_NONE)
    {
        return SharedItems[Index];
    }
    return FInventoryItem(); // Retorna item inválido
}

void UInventorySubsystem::MarkItemAsSeen(const FGuid& ItemID)
{
    if (!ItemID.IsValid())
    {
        return;
    }
    if (!bCacheValid)
    {
        RebuildCaches();
    }
    int32 Index = FindItemIndexByID(ItemID);
    if (Index != INDEX_NONE && SharedItems.IsValidIndex(Index))
    {
        if (SharedItems[Index].bIsNew)
        {
            SharedItems[Index].bIsNew = false;
            // Notificar UI (sem alterar quantidade)
            FInventoryItem ChangeNotification = SharedItems[Index];
            ChangeNotification.Quantity = 0;
            BroadcastInventoryChange(ChangeNotification);
        }
    }
}

TArray<FInventoryItem> UInventorySubsystem::GetItemsByFilter(EInventoryFilterCategory Filter) const
{
    TArray<FInventoryItem> Result;
    Result.Reserve(SharedItems.Num());
    for (const FInventoryItem& Item : SharedItems)
    {
        if (DoesItemMatchFilter(Item, Filter))
        {
            Result.Add(Item);
        }
    }
    return Result;
}

bool UInventorySubsystem::DoesItemMatchFilter(const FInventoryItem& Item, EInventoryFilterCategory Filter) const
{
    if (!Item.ItemData)
    {
        return false;
    }
    switch (Filter)
    {
    case EInventoryFilterCategory::None:
        return true;
    case EInventoryFilterCategory::NewItems:
        return Item.bIsNew;
    case EInventoryFilterCategory::Consumable:
        return Item.ItemData->ItemCategory == EItemCategory::Consumable;
    // Filtros de subtipo são tratados dinamicamente
    case EInventoryFilterCategory::Weapon:
        return Item.ItemData->ItemCategory == EItemCategory::Weapon;
    // Filtros de subtipo são tratados dinamicamente
    case EInventoryFilterCategory::Accessory:
        return Item.ItemData->ItemCategory == EItemCategory::Accessory;
    // Filtros de subtipo são tratados dinamicamente
    case EInventoryFilterCategory::Armor:
        return Item.ItemData->ItemCategory == EItemCategory::Armor;
    // Filtros de subtipo são tratados dinamicamente
    case EInventoryFilterCategory::Ring:
        return Item.ItemData->ItemCategory == EItemCategory::Ring;
    case EInventoryFilterCategory::Boots:
        return Item.ItemData->ItemCategory == EItemCategory::Boots;
    // Filtros de subtipo são tratados dinamicamente
    case EInventoryFilterCategory::Materials:
        return Item.ItemData->ItemCategory == EItemCategory::Material;
    case EInventoryFilterCategory::Passive:
        return Item.ItemData->ItemCategory == EItemCategory::Consumable && Item.ItemData->bIsPassive;
    case EInventoryFilterCategory::Quest:
        return Item.ItemData->ItemCategory == EItemCategory::Valuable;
    case EInventoryFilterCategory::Cosmetic:
        return Item.ItemData->ItemCategory == EItemCategory::Cosmetic;
    case EInventoryFilterCategory::Expansion:
        return Item.ItemData->ItemCategory == EItemCategory::Expansion;
    default:
        return false;
    }
}


// ----- Funções Privadas -----

int32 UInventorySubsystem::FindStackableItemIndex(UItemDataAsset* ItemData, int32 Level) const
{
    if (!ItemData || !ItemData->bIsStackable)
    {
        return INDEX_NONE;
    }

    for (int32 i = 0; i < SharedItems.Num(); ++i)
    {
        const FInventoryItem& Item = SharedItems[i];
        // Verifica se é o mesmo item, mesmo nível e se ainda há espaço na pilha
        if (Item.ItemData == ItemData && Item.Level == Level && Item.Quantity < ItemData->MaxStackSize)
        {
            return i;
        }
    }
    return INDEX_NONE;
}

int32 UInventorySubsystem::FindItemIndexByID(const FGuid& ItemID) const
{
    if (!ItemID.IsValid()) return INDEX_NONE;
    
    // OTIMIZAÇÃO: Usar cache se válido
    if (bCacheValid)
    {
        if (const int32* CachedIndex = ItemIndexCache.Find(ItemID))
        {
            // Verificar se o índice ainda é válido
            if (SharedItems.IsValidIndex(*CachedIndex) && SharedItems[*CachedIndex].UniqueID == ItemID)
            {
                return *CachedIndex;
            }
            // Cache inválido, forçar rebuild
            const_cast<UInventorySubsystem*>(this)->InvalidateCaches();
        }
        // Se não encontrou no cache, item não existe
        return INDEX_NONE;
    }
    
    // Sem fallback: garantir cache válido e consultar
    const_cast<UInventorySubsystem*>(this)->RebuildCaches();
    if (const int32* CachedIndex = ItemIndexCache.Find(ItemID))
    {
        if (SharedItems.IsValidIndex(*CachedIndex) && SharedItems[*CachedIndex].UniqueID == ItemID)
        {
            return *CachedIndex;
        }
    }
    return INDEX_NONE;
}

void UInventorySubsystem::BroadcastInventoryChange(const FInventoryItem& Item)
{
    // OTIMIZAÇÃO: Usar sistema de batch em vez de broadcast imediato
    AddToPendingChanges(Item);
}

// --- Fim Implementação Funções de Transferência --- 

// === IMPLEMENTAÇÕES DE OTIMIZAÇÃO ===

void UInventorySubsystem::RebuildCaches()
{
    UE_LOG(LogTemp, Verbose, TEXT("InventorySubsystem: Rebuilding performance caches..."));
    
    // Limpar caches existentes
    ItemIndexCache.Empty(SharedItems.Num());
    ItemCountCache.Empty();
    
    // Reconstruir cache de índices e contadores
    for (int32 i = 0; i < SharedItems.Num(); ++i)
    {
        const FInventoryItem& Item = SharedItems[i];
        
        // Cache de índice por ID
        ItemIndexCache.Add(Item.UniqueID, i);
        
        // Cache de contadores por tipo
        FString CacheKey = GenerateItemCacheKey(Item.ItemData, Item.Level);
        if (int32* ExistingCount = ItemCountCache.Find(CacheKey))
        {
            *ExistingCount += Item.Quantity;
        }
        else
        {
            ItemCountCache.Add(CacheKey, Item.Quantity);
        }
    }
    
    bCacheValid = true;
    UE_LOG(LogTemp, Verbose, TEXT("InventorySubsystem: Caches rebuilt. IndexCache: %d entries, CountCache: %d entries"), 
           ItemIndexCache.Num(), ItemCountCache.Num());
}

void UInventorySubsystem::InvalidateCaches()
{
    bCacheValid = false;
    // Não limpar os maps aqui - será feito no próximo RebuildCaches()
    UE_LOG(LogTemp, VeryVerbose, TEXT("InventorySubsystem: Caches invalidated"));
}

void UInventorySubsystem::UpdateItemCache(const FInventoryItem& Item, bool bAdding)
{
    if (!bCacheValid) return; // Se cache já inválido, não precisa atualizar
    
    // Atualizar cache de contadores
    FString CacheKey = GenerateItemCacheKey(Item.ItemData, Item.Level);
    
    if (bAdding)
    {
        // Adicionando item
        if (int32* ExistingCount = ItemCountCache.Find(CacheKey))
        {
            *ExistingCount += Item.Quantity;
        }
        else
        {
            ItemCountCache.Add(CacheKey, Item.Quantity);
        }
        
        // Adicionar ao cache de índices (será atualizado no próximo rebuild se necessário)
        // Por enquanto, invalidar cache de índices pois as posições mudaram
        InvalidateCaches();
    }
    else
    {
        // Removendo item
        if (int32* ExistingCount = ItemCountCache.Find(CacheKey))
        {
            *ExistingCount -= Item.Quantity;
            if (*ExistingCount <= 0)
            {
                ItemCountCache.Remove(CacheKey);
            }
        }
        
        // Invalidar cache de índices pois as posições mudaram
        InvalidateCaches();
    }
}

FString UInventorySubsystem::GenerateItemCacheKey(UItemDataAsset* ItemData, int32 Level) const
{
    if (!ItemData) return TEXT("");
    
    // Usar ponteiro do objeto + level como chave única
    return FString::Printf(TEXT("%p_%d"), ItemData, Level);
}

void UInventorySubsystem::ProcessPendingBroadcasts()
{
    if (PendingChanges.Num() == 0) return;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("InventorySubsystem: Processing %d pending broadcasts"), PendingChanges.Num());
    
    // Fazer broadcast de todas as mudanças pendentes
    for (const FInventoryItem& Item : PendingChanges)
    {
        EItemCategory Category = (Item.ItemData) ? Item.ItemData->ItemCategory : EItemCategory::None;
        OnInventoryChanged.Broadcast(Category, Item);
    }
    
    // Limpar mudanças pendentes
    PendingChanges.Empty();
    
    // Limpar timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BatchBroadcastTimer);
    }
}

void UInventorySubsystem::AddToPendingChanges(const FInventoryItem& Item)
{
    // Adicionar à lista de mudanças pendentes
    PendingChanges.Add(Item);
    
    // Configurar timer para processar no próximo tick (se ainda não configurado)
    if (UWorld* World = GetWorld())
    {
        if (!World->GetTimerManager().IsTimerActive(BatchBroadcastTimer))
        {
            World->GetTimerManager().SetTimer(
                BatchBroadcastTimer,
                this,
                &UInventorySubsystem::ProcessPendingBroadcasts,
                0.0f, // Processar no próximo tick
                false
            );
        }
    }
}

// === FIM IMPLEMENTAÇÕES DE OTIMIZAÇÃO ===

// === IMPLEMENTAÇÕES DE BENCHMARK/DEBUG ===

void UInventorySubsystem::BenchmarkInventoryPerformance(int32 NumOperations)
{
    UE_LOG(LogTemp, Warning, TEXT("=== BENCHMARK DO INVENTÁRIO ==="));
    UE_LOG(LogTemp, Warning, TEXT("Operações: %d"), NumOperations);
    
    // Limpar inventário para teste limpo
    SharedItems.Empty();
    InvalidateCaches();
    
    // Criar item de teste
    UItemDataAsset* TestItem = NewObject<UItemDataAsset>();
    TestItem->ItemName = FText::FromString(TEXT("TestItem"));
    TestItem->bIsStackable = true;
    TestItem->MaxStackSize = 99;
    TestItem->ItemCategory = EItemCategory::Material;
    
    // === TESTE 1: Adições ===
    double StartTime = FPlatformTime::Seconds();
    
    for (int32 i = 0; i < NumOperations; ++i)
    {
        AddItem(TestItem, 1, 1);
    }
    
    double AddTime = FPlatformTime::Seconds() - StartTime;
    UE_LOG(LogTemp, Warning, TEXT("Tempo para %d adições: %.4f segundos (%.2f ops/sec)"), 
           NumOperations, AddTime, NumOperations / AddTime);
    
    // === TESTE 2: Buscas ===
    StartTime = FPlatformTime::Seconds();
    
    int32 TotalFound = 0;
    for (int32 i = 0; i < NumOperations; ++i)
    {
        TotalFound += GetItemCount(TestItem, 1);
    }
    
    double SearchTime = FPlatformTime::Seconds() - StartTime;
    UE_LOG(LogTemp, Warning, TEXT("Tempo para %d buscas: %.4f segundos (%.2f ops/sec)"), 
           NumOperations, SearchTime, NumOperations / SearchTime);
    UE_LOG(LogTemp, Warning, TEXT("Total encontrado: %d"), TotalFound);
    
    // === TESTE 3: Buscas por ID ===
    if (SharedItems.Num() > 0)
    {
        FGuid TestID = SharedItems[0].UniqueID;
        StartTime = FPlatformTime::Seconds();
        
        int32 FoundCount = 0;
        for (int32 i = 0; i < NumOperations; ++i)
        {
            if (FindItemIndexByID(TestID) != INDEX_NONE)
            {
                FoundCount++;
            }
        }
        
        double IDSearchTime = FPlatformTime::Seconds() - StartTime;
        UE_LOG(LogTemp, Warning, TEXT("Tempo para %d buscas por ID: %.4f segundos (%.2f ops/sec)"), 
               NumOperations, IDSearchTime, NumOperations / IDSearchTime);
        UE_LOG(LogTemp, Warning, TEXT("IDs encontrados: %d"), FoundCount);
    }
    
    // === ESTATÍSTICAS FINAIS ===
    UE_LOG(LogTemp, Warning, TEXT("Itens no inventário: %d"), SharedItems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Cache Stats: %s"), *GetCacheStats());
    UE_LOG(LogTemp, Warning, TEXT("=== FIM DO BENCHMARK ==="));
}

FString UInventorySubsystem::GetCacheStats() const
{
    return FString::Printf(TEXT("Cache Válido: %s | Índices: %d | Contadores: %d | Broadcasts Pendentes: %d"),
                          bCacheValid ? TEXT("SIM") : TEXT("NÃO"),
                          ItemIndexCache.Num(),
                          ItemCountCache.Num(),
                          PendingChanges.Num());
}

// === FIM IMPLEMENTAÇÕES DE BENCHMARK/DEBUG ===

// === SISTEMA DE GOLD ===

void UInventorySubsystem::AddGold(int32 Amount)
{
    if (Amount <= 0)
    {
        return;
    }

    int32 OldGold = CurrentGold;
    CurrentGold += Amount;
    
    // Garantir que não fique negativo (por segurança)
    if (CurrentGold < 0)
    {
        CurrentGold = 0;
    }

    // Disparar evento apenas se realmente mudou
    if (CurrentGold != OldGold)
    {
        OnGoldChanged.Broadcast(CurrentGold);
        UE_LOG(LogTemp, Log, TEXT("SharedInventory: Gold adicionado - %d -> %d (+%d)"), OldGold, CurrentGold, Amount);
    }
}

bool UInventorySubsystem::RemoveGold(int32 Amount)
{
    if (Amount <= 0)
    {
        return true; // Remover 0 é sempre sucesso
    }

    if (CurrentGold < Amount)
    {
        UE_LOG(LogTemp, Warning, TEXT("SharedInventory: Tentativa de remover %d gold, mas só tem %d"), Amount, CurrentGold);
        return false;
    }

    int32 OldGold = CurrentGold;
    CurrentGold -= Amount;
    
    // Disparar evento
    OnGoldChanged.Broadcast(CurrentGold);
    UE_LOG(LogTemp, Log, TEXT("SharedInventory: Gold removido - %d -> %d (-%d)"), OldGold, CurrentGold, Amount);
    
    return true;
}

void UInventorySubsystem::SetGold(int32 NewAmount)
{
    if (NewAmount < 0)
    {
        NewAmount = 0;
    }

    if (CurrentGold != NewAmount)
    {
        int32 OldGold = CurrentGold;
        CurrentGold = NewAmount;
        
        // Disparar evento
        OnGoldChanged.Broadcast(CurrentGold);
        UE_LOG(LogTemp, Log, TEXT("SharedInventory: Gold definido - %d -> %d"), OldGold, CurrentGold);
    }
}

TArray<FInventoryItem> UInventorySubsystem::GetItemsForClass(EPlayerClass PlayerClass) const
{
    TArray<FInventoryItem> Result;
    Result.Reserve(SharedItems.Num());
    
    for (const FInventoryItem& Item : SharedItems)
    {
        if (Item.ItemData && Item.ItemData->CanClassUse(PlayerClass))
        {
            Result.Add(Item);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("GetItemsForClass: Filtrados %d itens para classe %d (total: %d)"), 
        Result.Num(), (int32)PlayerClass, SharedItems.Num());
    
    return Result;
}

// === FIM SISTEMA DE GOLD ===
