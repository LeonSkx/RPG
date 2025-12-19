#include "Inventory/Data/CategoryIconsDataAsset.h"

UTexture2D* UCategoryIconsDataAsset::GetIconForCategory(EItemCategory Category) const
{
	switch (Category)
	{
		case EItemCategory::Weapon:
			return WeaponIcon;
		case EItemCategory::Armor:
			return ArmorIcon;
		case EItemCategory::Boots:
			return BootsIcon;
		case EItemCategory::Accessory:
			return AccessoryIcon;
		case EItemCategory::Ring:
			return RingIcon;
		case EItemCategory::Consumable:
			return ConsumableIcon;
		case EItemCategory::Material:
			return MaterialIcon;
		case EItemCategory::Valuable:
			return ValuableIcon;
		case EItemCategory::Cosmetic:
			return CosmeticIcon;
		case EItemCategory::Expansion:
			return ExpansionIcon;
		default:
			return nullptr;
	}
}


