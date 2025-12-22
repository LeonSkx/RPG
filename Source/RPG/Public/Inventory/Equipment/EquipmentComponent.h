#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Inventory/Core/InventoryEnums.h"
#include "Inventory/Core/InventoryTypes.h"
#include "EquipmentComponent.generated.h"

class UEquippedItem;

// Slots de equipamento (alinhados com a UI do inventário)
UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
    Weapon    UMETA(DisplayName="Arma"),
    Armor     UMETA(DisplayName="Armadura"),
    Accessory UMETA(DisplayName="Acessório"),
    Boots     UMETA(DisplayName="Bota"),
    Ring      UMETA(DisplayName="Anel")
};

// Configuração de sockets para cada slot
USTRUCT(BlueprintType)
struct FSocketMapping
{
    GENERATED_BODY()

    // Lista de nomes de sockets (primeiro = principal, seguintes = alternativos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment|Sockets")
    TArray<FName> SocketNames;

    FSocketMapping()
    {
    }

    FSocketMapping(const TArray<FName>& InSocketNames)
        : SocketNames(InSocketNames) {}
};


// Delegates para eventos de equipar/desequipar (atualizados)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquippedSignature, EEquipmentSlot, Slot, UEquippedItem*, EquippedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemUnequippedSignature, EEquipmentSlot, Slot, const FInventoryItem&, UnequippedItem);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API UEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEquipmentComponent();
    virtual void BeginPlay() override;

    // --- Funções Principais de Equipamento ---

    /**
     * Equipa um item do inventário no slot indicado
     * @param InventoryItem Item do inventário a ser equipado
     * @param Slot Slot onde equipar o item
     * @return true se equipado com sucesso
     */
    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool EquipItemFromInventory(const FInventoryItem& InventoryItem, EEquipmentSlot Slot);

    /**
     * Desequipa o item no slot indicado
     * @param Slot Slot a ser desequipado
     * @return Item que estava equipado (para retornar ao inventário)
     */
    UFUNCTION(BlueprintCallable, Category="Equipment")
    FInventoryItem UnequipItem(EEquipmentSlot Slot);

    /**
     * Retorna o item equipado no slot
     */
    UFUNCTION(BlueprintPure, Category="Equipment")
    UEquippedItem* GetEquippedItem(EEquipmentSlot Slot) const;

    /**
     * Verifica se um slot está ocupado
     */
    UFUNCTION(BlueprintPure, Category="Equipment")
    bool IsSlotOccupied(EEquipmentSlot Slot) const;

    /**
     * Retorna todos os itens equipados
     */
    UFUNCTION(BlueprintPure, Category="Equipment")
    TArray<UEquippedItem*> GetAllEquippedItems() const;

    /**
     * Atualiza o nível de um item equipado e reaplica os efeitos
     */
    UFUNCTION(BlueprintCallable, Category="Equipment")
    void UpgradeEquippedItem(EEquipmentSlot Slot, int32 NewLevel);

    // --- Funções de Socket ---

    /**
     * Configura os sockets para um slot (primeiro = principal, seguintes = alternativos)
     */
    UFUNCTION(BlueprintCallable, Category="Equipment|Sockets")
    void SetSocketNamesForSlot(EEquipmentSlot Slot, const TArray<FName>& SocketNames);

    // Retorna o principal + alternativos (na ordem), como FNames. Vazio se nenhum definido
    UFUNCTION(BlueprintPure, Category="Equipment|Sockets")
    TArray<FName> GetAllSocketNamesForSlot(EEquipmentSlot Slot) const;

    

    // --- Funções Legacy (compatibilidade com sistema antigo) ---

    /**
     * @deprecated Use EquipItemFromInventory instead
     */
    UFUNCTION(BlueprintCallable, Category="Equipment", meta = (DeprecatedFunction, DeprecationMessage = "Use EquipItemFromInventory instead"))
    bool EquipItem(UItemDataAsset* Item, EEquipmentSlot Slot);

    /**
     * @deprecated Use GetEquippedItem()->GetItemData() instead
     */
    UFUNCTION(BlueprintPure, Category="Equipment", meta = (DeprecatedFunction, DeprecationMessage = "Use GetEquippedItem()->GetItemData() instead"))
    UItemDataAsset* GetEquippedItemData(EEquipmentSlot Slot) const;

    // --- Eventos ---

    UPROPERTY(BlueprintAssignable, Category="Equipment")
    FOnItemEquippedSignature OnItemEquipped;

    UPROPERTY(BlueprintAssignable, Category="Equipment")
    FOnItemUnequippedSignature OnItemUnequipped;

protected:
    // Itens atualmente equipados por slot
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Equipment")
    TMap<EEquipmentSlot, TObjectPtr<UEquippedItem>> EquippedItems;

    // Mapeamento de slots para sockets do personagem
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Equipment|Sockets")
    TMap<EEquipmentSlot, FSocketMapping> SocketMappings;

    

private:
    /**
     * Inicializa mapeamentos padrão de sockets
     */
    void InitializeDefaultSocketMappings();

    /**
     * Valida se um item pode ser equipado em um slot
     */
    bool CanEquipItemInSlot(const FInventoryItem& Item, EEquipmentSlot Slot) const;

    /**
     * Determina automaticamente o slot para um item baseado no seu tipo
     */
    EEquipmentSlot DetermineSlotForItem(const FInventoryItem& Item) const;
}; 
