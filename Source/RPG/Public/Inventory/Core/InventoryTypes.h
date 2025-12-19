#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/InventoryEnums.h"
#include "InventoryTypes.generated.h"

// Declaração antecipada
class UItemDataAsset;

/**
 * Representa um item individual no inventário
 */
USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    // Referência para o data asset do item
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    TObjectPtr<UItemDataAsset> ItemData = nullptr;

    // Quantidade do item
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
    int32 Quantity = 1;

    // Nível do item (para itens com níveis)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = "1"))
    int32 Level = 1;

    // ID único para esse item específico (para itens únicos)
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    FGuid UniqueID;

    // Marca visual/UX: item recém-adicionado (para filtro "Novo")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    bool bIsNew = false;

    // Construtor padrão
    FInventoryItem()
        : ItemData(nullptr)
        , Quantity(1)
        , Level(1)
        , UniqueID(FGuid::NewGuid())
        , bIsNew(false)
    {
    }

    // Construtor com parâmetros
    FInventoryItem(UItemDataAsset* InItemData, int32 InQuantity = 1, int32 InLevel = 1)
        : ItemData(InItemData)
        , Quantity(InQuantity)
        , Level(InLevel)
        , UniqueID(FGuid::NewGuid())
        , bIsNew(true)
    {
    }

    // Verificar se o item é válido
    bool IsValid() const
    {
        return ItemData != nullptr && Quantity > 0;
    }

    // Comparação entre itens (ignora quantidade)
    bool operator==(const FInventoryItem& Other) const
    {
        return ItemData == Other.ItemData && Level == Other.Level;
    }
};

/**
 * Representa uma categoria específica do inventário com seus itens
 */
USTRUCT(BlueprintType)
struct FInventoryCategory
{
    GENERATED_BODY()

    // Tipo da categoria
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    EItemCategory CategoryType = EItemCategory::None;

    // Itens nesta categoria
    UPROPERTY(BlueprintReadOnly, Category = "Inventory")
    TArray<FInventoryItem> Items;

    // Capacidade máxima desta categoria (-1 = ilimitado)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 MaxCapacity = -1;

    // Verifica se a categoria tem espaço para mais itens
    bool HasSpace() const
    {
        return MaxCapacity < 0 || Items.Num() < MaxCapacity;
    }

    // Verifica se a categoria tem espaço para N novos itens
    bool HasSpaceFor(int32 NumItems) const
    {
        return MaxCapacity < 0 || (Items.Num() + NumItems) <= MaxCapacity;
    }
};

/**
 * Configurações para os limites de cada categoria do inventário
 */
USTRUCT(BlueprintType)
struct FInventoryCapacitySettings
{
    GENERATED_BODY()

    // Capacidade máxima geral do inventário
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ClampMin = "-1"))
    int32 GlobalMaxCapacity = -1;

    // Capacidades por categoria (se não especificada, usa GlobalMaxCapacity)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<EItemCategory, int32> CategoryCapacities;

    // Obtém o limite para uma categoria específica
    int32 GetCapacityForCategory(EItemCategory Category) const
    {
        if (CategoryCapacities.Contains(Category))
        {
            return CategoryCapacities[Category];
        }
        return GlobalMaxCapacity;
    }
};

/**
 * Resultado de uma operação de inventário
 */
UENUM(BlueprintType)
enum class EInventoryActionResult : uint8
{
    Success             UMETA(DisplayName = "Sucesso"),
    Failed_InvalidItem  UMETA(DisplayName = "Falha: Item Inválido"),
    Failed_NoSpace      UMETA(DisplayName = "Falha: Sem Espaço"),
    Failed_NotFound     UMETA(DisplayName = "Falha: Item Não Encontrado"),
    Failed_NotEnough    UMETA(DisplayName = "Falha: Quantidade Insuficiente"),
    Failed_Generic      UMETA(DisplayName = "Falha: Erro Genérico")
};

/**
 * Evento disparado quando o inventário muda
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, EItemCategory, Category, const FInventoryItem&, Item);

/**
 * Evento disparado quando há mudança em moeda/ouro
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewAmount); 
