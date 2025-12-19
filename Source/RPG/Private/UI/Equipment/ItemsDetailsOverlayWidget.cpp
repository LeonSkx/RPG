#include "UI/Equipment/ItemsDetailsOverlayWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Inventory/Core/InventoryEnums.h"
#include "UI/UIUtilities.h"

void UItemsDetailsOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Configurar botão de fechar se existir
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveAll(this);
		CloseButton->OnClicked.AddDynamic(this, &UItemsDetailsOverlayWidget::HandleCloseButtonClicked);
	}

	// Inicialmente oculto
	SetVisibility(ESlateVisibility::Hidden);
}

void UItemsDetailsOverlayWidget::Setup(const FInventoryItem& InItem)
{
	CurrentItem = InItem;
	
	if (InItem.IsValid())
	{
		UpdateDisplay();
		SetVisibility(ESlateVisibility::Visible);
		OnSelected.Broadcast();
	}
	else
	{
		Clear();
	}
}

void UItemsDetailsOverlayWidget::Clear()
{
	CurrentItem = FInventoryItem();
	
	if (ItemNameText)
	{
		ItemNameText->SetText(FText::FromString(TEXT("")));
	}
	
	if (ItemTypeText)
	{
		ItemTypeText->SetText(FText::FromString(TEXT("")));
	}

	if (ItemDescriptionText)
	{
		ItemDescriptionText->SetText(FText::FromString(TEXT("")));
	}

	if (ItemTypeIcon)
	{
		ItemTypeIcon->SetBrushFromTexture(nullptr);
	}
	
	SetVisibility(ESlateVisibility::Hidden);
}

void UItemsDetailsOverlayWidget::UpdateDisplay()
{
	if (!CurrentItem.IsValid() || !CurrentItem.ItemData)
	{
		Clear();
		return;
	}

	// Atualizar nome do equipamento
	if (ItemNameText)
	{
		ItemNameText->SetText(CurrentItem.ItemData->ItemName);
	}

	// Atualizar tipo/categoria do equipamento
	if (ItemTypeText)
	{
		FString TypeString = GetEquipmentTypeDisplayString(CurrentItem.ItemData);
		ItemTypeText->SetText(FText::FromString(TypeString));
	}

	// Atualizar descrição do equipamento
	if (ItemDescriptionText)
	{
		ItemDescriptionText->SetText(CurrentItem.ItemData->Description);
	}

	// Atualizar ícone do tipo do equipamento
	if (ItemTypeIcon && CurrentItem.ItemData->TypeIcon.LoadSynchronous())
	{
		UTexture2D* IconTexture = CurrentItem.ItemData->TypeIcon.LoadSynchronous();
		
		// Usar tamanho padrão
		ItemTypeIcon->SetBrushFromTexture(IconTexture, true);
	}
}

FString UItemsDetailsOverlayWidget::GetEquipmentTypeDisplayString(const UItemDataAsset* ItemData) const
{
	if (!ItemData)
	{
		return TEXT("");
	}

	// Sistema dinâmico: obtém o nome do subtipo baseado no enum
	switch (ItemData->ItemCategory)
	{
	case EItemCategory::Weapon:
		return UIUtilities::GetEnumDisplayName(ItemData->WeaponType);
		
	case EItemCategory::Accessory:
		return UIUtilities::GetEnumDisplayName(ItemData->AccessoryType);
		
	case EItemCategory::Ring:
		return UIUtilities::GetEnumDisplayName(ItemData->RingType);
		
	case EItemCategory::Armor:
		return UIUtilities::GetEnumDisplayName(ItemData->ArmorType);
		
	case EItemCategory::Boots:
		return UIUtilities::GetEnumDisplayName(ItemData->BootsType);
		
	default:
		return TEXT("");
	}
}

void UItemsDetailsOverlayWidget::HandleCloseButtonClicked()
{
	Clear();
}
