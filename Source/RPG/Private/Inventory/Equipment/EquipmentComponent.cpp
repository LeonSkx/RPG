#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquippedItem.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Interaction/CombatInterface.h"
#include "Character/RPGCharacter.h"
#include "Character/PlayerClassInfo.h"

UEquipmentComponent::UEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar mapeamentos padrão de sockets SEM sobrescrever o que foi configurado no Editor
    InitializeDefaultSocketMappings();
}

bool UEquipmentComponent::EquipItemFromInventory(const FInventoryItem& InventoryItem, EEquipmentSlot Slot)
{
    // Validação inicial do item
    if (!InventoryItem.IsValid() || !InventoryItem.ItemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Item inválido ou ItemData é nullptr"));
        return false;
    }

    // Validação do Owner
    AActor* Owner = GetOwner();
    if (!IsValid(Owner))
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Owner inválido"));
        return false;
    }

    // Obter nível do personagem via ICombatInterface (reduz acoplamento)
    int32 CharacterLevel = 1;
    if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Owner))
    {
        CharacterLevel = ICombatInterface::Execute_GetCharacterLevel(Owner);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Owner não implementa ICombatInterface"));
        return false;
    }

    // Verificar o nível requerido
    if (CharacterLevel < InventoryItem.ItemData->RequiredLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Nível insuficiente: Personagem=%d, Requerido=%d"), 
            CharacterLevel, InventoryItem.ItemData->RequiredLevel);
        return false;
    }

    // Verificar restrição de classe (se aplicável)
    if (InventoryItem.ItemData->AllowedClasses.Num() > 0)
    {
        // Tentar obter PlayerClass via cast para ARPGCharacter (melhor que buscar Controller)
        const ARPGCharacter* RPGCharacter = Cast<ARPGCharacter>(Owner);
        if (RPGCharacter)
        {
            EPlayerClass CharacterClass = RPGCharacter->GetPlayerClass();
            if (!InventoryItem.ItemData->CanClassUse(CharacterClass))
            {
                UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Personagem classe '%d' não pode usar este item (AllowedClasses: %d)"), 
                    (int32)CharacterClass, InventoryItem.ItemData->AllowedClasses.Num());
                return false;
            }
        }
    }
    
    // Validar se o item pode ser equipado neste slot
    if (!CanEquipItemInSlot(InventoryItem, Slot))
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Item categoria '%d' não pode ser equipado no slot '%d'"), 
            (int32)InventoryItem.ItemData->ItemCategory, (int32)Slot);
        return false;
    }

    // Se já há algo equipado neste slot, desequipar primeiro
    if (IsSlotOccupied(Slot))
    {
        UnequipItem(Slot);
    }

    // Criar novo item equipado
    UEquippedItem* NewEquippedItem = NewObject<UEquippedItem>(this);
    if (!NewEquippedItem)
    {
        UE_LOG(LogTemp, Error, TEXT("[EquipItem] Falha ao criar UEquippedItem"));
        return false;
    }

    // Inicializar o item equipado
    NewEquippedItem->InitializeFromInventoryItem(InventoryItem, Slot);

    // Aplicar stats do item ao personagem (com validação)
    if (IsValid(Owner))
    {
        NewEquippedItem->ApplyStatsToCharacter(Owner);
    }

    // Armazenar o item equipado
    EquippedItems.Add(Slot, NewEquippedItem);

    // Disparar evento
    OnItemEquipped.Broadcast(Slot, NewEquippedItem);

    return true;
}

FInventoryItem UEquipmentComponent::UnequipItem(EEquipmentSlot Slot)
{
    TObjectPtr<UEquippedItem>* FoundItem = EquippedItems.Find(Slot);
    if (!FoundItem || !(*FoundItem))
    {
        return FInventoryItem(); // Item inválido
    }

    UEquippedItem* ItemToUnequip = *FoundItem;

    // Converter de volta para FInventoryItem
    FInventoryItem UnequippedInventoryItem = ItemToUnequip->ConvertBackToInventoryItem();

    // Remover stats do personagem (com validação)
    AActor* Owner = GetOwner();
    if (IsValid(Owner))
    {
        ItemToUnequip->RemoveStatsFromCharacter(Owner);
    }

    // Desanexar visualmente
    ItemToUnequip->DetachFromSocket();

    // Remover do mapa
    EquippedItems.Remove(Slot);

    // Disparar evento
    OnItemUnequipped.Broadcast(Slot, UnequippedInventoryItem);

    // Limpar o objeto (será coletado pelo GC)
    ItemToUnequip = nullptr;

    return UnequippedInventoryItem;
}

UEquippedItem* UEquipmentComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
    const TObjectPtr<UEquippedItem>* FoundItem = EquippedItems.Find(Slot);
    return FoundItem ? *FoundItem : nullptr;
}

bool UEquipmentComponent::IsSlotOccupied(EEquipmentSlot Slot) const
{
    return EquippedItems.Contains(Slot) && EquippedItems[Slot] != nullptr;
}

TArray<UEquippedItem*> UEquipmentComponent::GetAllEquippedItems() const
{
    TArray<UEquippedItem*> AllItems;
    
    for (const auto& ItemPair : EquippedItems)
    {
        if (ItemPair.Value)
        {
            AllItems.Add(ItemPair.Value);
        }
    }
    
    return AllItems;
}

void UEquipmentComponent::SetSocketNamesForSlot(EEquipmentSlot Slot, const TArray<FName>& SocketNames)
{
    SocketMappings.Add(Slot, FSocketMapping(SocketNames));
}

void UEquipmentComponent::UpgradeEquippedItem(EEquipmentSlot Slot, int32 NewLevel)
{
    UEquippedItem* EquippedItem = GetEquippedItem(Slot);
    if (!EquippedItem)
    {
        UE_LOG(LogTemp, Warning, TEXT("UEquipmentComponent::UpgradeEquippedItem - Nenhum item equipado no slot %s"), 
            *UEnum::GetValueAsString(Slot));
        return;
    }

    // Fazer upgrade do item equipado
    EquippedItem->UpgradeItem(NewLevel);
    
    UE_LOG(LogTemp, Log, TEXT("UEquipmentComponent::UpgradeEquippedItem - Item no slot %s atualizado para Level %d"), 
        *UEnum::GetValueAsString(Slot), NewLevel);
}

TArray<FName> UEquipmentComponent::GetAllSocketNamesForSlot(EEquipmentSlot Slot) const
{
    TArray<FName> Out;
    if (const FSocketMapping* Mapping = SocketMappings.Find(Slot))
    {
        for (const FName& Name : Mapping->SocketNames)
        {
            if (!Name.IsNone())
            {
                Out.Add(Name);
            }
        }
    }
    return Out;
}

// removed AddSocketNameForSlot / GetSocketNamesForSlot (revert to single-name mapping)

// --- Funções Legacy (compatibilidade) ---

bool UEquipmentComponent::EquipItem(UItemDataAsset* Item, EEquipmentSlot Slot)
{
    if (!Item)
    {
        return false;
    }

    // Converter para FInventoryItem
    FInventoryItem InventoryItem(Item, 1, 1);
    
    // Usar nova função
    return EquipItemFromInventory(InventoryItem, Slot);
}

UItemDataAsset* UEquipmentComponent::GetEquippedItemData(EEquipmentSlot Slot) const
{
    UEquippedItem* EquippedItem = GetEquippedItem(Slot);
    return EquippedItem ? EquippedItem->GetItemData() : nullptr;
}

// --- Funções Privadas ---

void UEquipmentComponent::InitializeDefaultSocketMappings()
{
    // Configurar mapeamentos padrão para JRPG
    // Estes podem ser sobrescritos via Blueprint ou chamadas de função
    // Só definir se a entrada NÃO existir ainda (não sobrescrever valores do Editor)
    
    if (!SocketMappings.Contains(EEquipmentSlot::Weapon))
    {
        SetSocketNamesForSlot(EEquipmentSlot::Weapon, { FName("WeaponSocket"), FName("RightHandSocket") });
    }
    
    if (!SocketMappings.Contains(EEquipmentSlot::Armor))
    {
        SetSocketNamesForSlot(EEquipmentSlot::Armor, { FName("ArmorSocket"), FName("ChestSocket") });
    }
    
    if (!SocketMappings.Contains(EEquipmentSlot::Accessory))
    {
        SetSocketNamesForSlot(EEquipmentSlot::Accessory, { FName("AccessorySocket"), FName("NeckSocket") });
    }
    
    if (!SocketMappings.Contains(EEquipmentSlot::Boots))
    {
        SetSocketNamesForSlot(EEquipmentSlot::Boots, { FName("BootsSocket"), FName("FeetSocket") });
    }
    
    if (!SocketMappings.Contains(EEquipmentSlot::Ring))
    {
        SetSocketNamesForSlot(EEquipmentSlot::Ring, { FName("RingSocket"), FName("LeftHandSocket") });
    }
}

bool UEquipmentComponent::CanEquipItemInSlot(const FInventoryItem& Item, EEquipmentSlot Slot) const
{
    if (!Item.IsValid() || !Item.ItemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CanEquipItemInSlot] Item inválido ou ItemData é nullptr"));
        return false;
    }

    // Map estático para mapear categoria para slot (simplifica switch cases)
    static const TMap<EItemCategory, EEquipmentSlot> CategoryToSlotMap = {
        {EItemCategory::Weapon, EEquipmentSlot::Weapon},
        {EItemCategory::Armor, EEquipmentSlot::Armor},
        {EItemCategory::Accessory, EEquipmentSlot::Accessory},
        {EItemCategory::Boots, EEquipmentSlot::Boots},
        {EItemCategory::Ring, EEquipmentSlot::Ring}
    };

    // Verificar se é um item equipável por categoria
    const EItemCategory Cat = Item.ItemData->ItemCategory;
    if (!CategoryToSlotMap.Contains(Cat))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CanEquipItemInSlot] Categoria '%d' não é equipável"), (int32)Cat);
        return false;
    }

    // Verificar compatibilidade do tipo com o slot usando o map
    const EEquipmentSlot* ExpectedSlot = CategoryToSlotMap.Find(Cat);
    if (!ExpectedSlot)
    {
        return false;
    }

    const bool bCanEquip = (*ExpectedSlot == Slot);
    
    if (!bCanEquip)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CanEquipItemInSlot] Categoria '%d' não corresponde ao slot '%d'"), 
            (int32)Item.ItemData->ItemCategory, (int32)Slot);
    }

    return bCanEquip;
}

EEquipmentSlot UEquipmentComponent::DetermineSlotForItem(const FInventoryItem& Item) const
{
    if (!Item.IsValid() || !Item.ItemData)
    {
        return EEquipmentSlot::Weapon; // Default
    }

    // Map estático para mapear categoria para slot (reutiliza a mesma lógica)
    static const TMap<EItemCategory, EEquipmentSlot> CategoryToSlotMap = {
        {EItemCategory::Weapon, EEquipmentSlot::Weapon},
        {EItemCategory::Armor, EEquipmentSlot::Armor},
        {EItemCategory::Accessory, EEquipmentSlot::Accessory},
        {EItemCategory::Boots, EEquipmentSlot::Boots},
        {EItemCategory::Ring, EEquipmentSlot::Ring}
    };

    // Determinar slot baseado no tipo do item usando o map
    const EEquipmentSlot* FoundSlot = CategoryToSlotMap.Find(Item.ItemData->ItemCategory);
    return FoundSlot ? *FoundSlot : EEquipmentSlot::Weapon; // Default se não encontrado
} 
