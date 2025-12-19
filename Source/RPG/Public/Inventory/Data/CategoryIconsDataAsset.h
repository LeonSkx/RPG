#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Inventory/Core/InventoryEnums.h"
#include "CategoryIconsDataAsset.generated.h"

/**
 * Data Asset para gerenciar ícones das categorias de inventário
 * Permite configurar ícones personalizados para cada categoria
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API UCategoryIconsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Ícone para categoria Weapon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* WeaponIcon = nullptr;

	/** Ícone para categoria Armor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* ArmorIcon = nullptr;

	/** Ícone para categoria Boots */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* BootsIcon = nullptr;

	/** Ícone para categoria Accessory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* AccessoryIcon = nullptr;

	/** Ícone para categoria Ring */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* RingIcon = nullptr;

	/** Ícone para categoria Consumable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* ConsumableIcon = nullptr;

	/** Ícone para categoria Material */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* MaterialIcon = nullptr;

	/** Ícone para categoria Valuable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* ValuableIcon = nullptr;

	/** Ícone para categoria Cosmetic */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* CosmeticIcon = nullptr;

	/** Ícone para categoria Expansion */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category Icons")
	UTexture2D* ExpansionIcon = nullptr;

	// === ÍCONES ESPECIAIS ===
	
	/** Ícone para o botão "New Items" */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Icons")
	UTexture2D* NewIcon = nullptr;

	/** Ícone para o botão "All" dos subtypes (usado em todas as categorias) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Special Icons")
	UTexture2D* AllIcon = nullptr;

public:
	/** Obtém o ícone para uma categoria específica */
	UFUNCTION(BlueprintPure, Category = "Category Icons")
	UTexture2D* GetIconForCategory(EItemCategory Category) const;

	/** Obtém o ícone para uma categoria específica (versão Blueprint) */
	UFUNCTION(BlueprintPure, Category = "Category Icons")
	UTexture2D* GetIconForCategoryBP(EItemCategory Category) const { return GetIconForCategory(Category); }

	/** Obtém o ícone "All" para subtypes (usado em todas as categorias) */
	UFUNCTION(BlueprintPure, Category = "Category Icons")
	UTexture2D* GetAllIcon() const { return AllIcon; }

	/** Obtém o ícone para o botão "New Items" */
	UFUNCTION(BlueprintPure, Category = "Category Icons")
	UTexture2D* GetNewIcon() const { return NewIcon; }
};
