#include "Inventory/Items/ItemDataAsset.h"

UItemDataAsset::UItemDataAsset()
{
    // Inicializar valores padrão
    ItemName = FText::FromString("New Item");
    Description = FText::FromString("Description for new item");
    ItemCategory = EItemCategory::None;
    
    RequiredLevel = 1;
    bIsStackable = true;
    MaxStackSize = 99;
    bCanBeDiscarded = true;
    bIsRare = false;
}

bool UItemDataAsset::CanClassUse(EPlayerClass PlayerClass) const
{
    // Se não há restrições (array vazio), qualquer classe pode usar
    if (AllowedClasses.Num() == 0)
    {
        return true;
    }
    
    // Verificar se a classe está na lista de classes permitidas
    return AllowedClasses.Contains(PlayerClass);
} 
