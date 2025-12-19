#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquippedItem.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Game/RPGGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Player/RPGPlayerController.h"
#include "Progression/ProgressionSubsystem.h"
#include "Character/RPGCharacter.h"

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
    if (!InventoryItem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Item inválido"));
        return false;
    }

    if (!InventoryItem.ItemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] ItemData é nullptr"));
        return false;
    }

    // Obter o nível do personagem
    URPGGameInstance* GameInstance = Cast<URPGGameInstance>(UGameplayStatics::GetGameInstance(this));
    if (!GameInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] GameInstance não encontrado"));
        return false;
    }
    UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>();
    if (!ProgressionSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] ProgressionSubsystem não encontrado"));
        return false;
    }
    AController* Controller = UGameplayStatics::GetPlayerController(this, 0); // Supondo um único jogador
    if (!Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Controller não encontrado"));
        return false;
    }

    const ARPGCharacter* Character = Cast<ARPGCharacter>(Controller->GetPawn());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Character não encontrado ou não é ARPGCharacter"));
        return false;
    }

    // Verificar restrição de classe
    if (InventoryItem.ItemData->AllowedClasses.Num() > 0)
    {
        EPlayerClass CharacterClass = Character->GetPlayerClass();
        if (!InventoryItem.ItemData->CanClassUse(CharacterClass))
        {
            UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Personagem classe '%d' não pode usar este item (AllowedClasses: %d)"), 
                (int32)CharacterClass, InventoryItem.ItemData->AllowedClasses.Num());
            return false;
        }
    }

    const int32 PlayerLevel = ProgressionSubsystem->GetCharacterLevel(Character);

    // Verificar o nível requerido
    if (PlayerLevel < InventoryItem.ItemData->RequiredLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EquipItem] Nível insuficiente: Personagem=%d, Requerido=%d"), 
            PlayerLevel, InventoryItem.ItemData->RequiredLevel);
        return false;
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

    // Aplicar stats do item ao personagem
    NewEquippedItem->ApplyStatsToCharacter(GetOwner());

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

    // Remover stats do personagem
    ItemToUnequip->RemoveStatsFromCharacter(GetOwner());

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

void UEquipmentComponent::SetSocketNamesForSlot(EEquipmentSlot Slot, const TArray<FName>& SocketNames, const FTransform& Offset)
{
    SocketMappings.Add(Slot, FSocketMapping(SocketNames, Offset));
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

    // Verificar se é um item equipável por categoria
    const EItemCategory Cat = Item.ItemData->ItemCategory;
    const bool bEquipCategory = (Cat == EItemCategory::Weapon || Cat == EItemCategory::Accessory || Cat == EItemCategory::Ring || Cat == EItemCategory::Armor || Cat == EItemCategory::Boots);
    if (!bEquipCategory)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CanEquipItemInSlot] Categoria '%d' não é equipável"), (int32)Cat);
        return false;
    }

    // Verificar compatibilidade do tipo com o slot
    bool bCanEquip = false;
    switch (Slot)
    {
        case EEquipmentSlot::Weapon:
            bCanEquip = Item.ItemData->ItemCategory == EItemCategory::Weapon;
            break;
            
        case EEquipmentSlot::Armor:
            bCanEquip = Item.ItemData->ItemCategory == EItemCategory::Armor;
            break;
            
        case EEquipmentSlot::Accessory:
            bCanEquip = Item.ItemData->ItemCategory == EItemCategory::Accessory;
            break;
            
        case EEquipmentSlot::Boots:
            bCanEquip = Item.ItemData->ItemCategory == EItemCategory::Boots;
            break;
            
        case EEquipmentSlot::Ring:
            bCanEquip = Item.ItemData->ItemCategory == EItemCategory::Ring;
            break;
            
        default:
            UE_LOG(LogTemp, Warning, TEXT("[CanEquipItemInSlot] Slot inválido: %d"), (int32)Slot);
            return false;
    }

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

    // Determinar slot baseado no tipo do item
    switch (Item.ItemData->ItemCategory)
    {
        case EItemCategory::Weapon:
            return EEquipmentSlot::Weapon;
            
        case EItemCategory::Armor:
            return EEquipmentSlot::Armor;
            
        case EItemCategory::Accessory:
            return EEquipmentSlot::Accessory;
            
        case EItemCategory::Boots:
            return EEquipmentSlot::Boots;
            
        case EItemCategory::Ring:
            return EEquipmentSlot::Ring;
            
        default:
            return EEquipmentSlot::Weapon; // Default
    }
} 
