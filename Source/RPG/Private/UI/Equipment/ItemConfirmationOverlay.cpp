#include "UI/Equipment/ItemConfirmationOverlay.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Character/RPGCharacter.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquippedItem.h"
#include "Inventory/Core/InventorySubsystem.h"
#include "Game/RPGGameModeBase.h"
#include "Kismet/GameplayStatics.h"

void UItemConfirmationOverlay::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetupButtonEvents();
}

void UItemConfirmationOverlay::SetupConfirmation(ARPGCharacter* CurrentUser, ARPGCharacter* InTargetCharacter, UItemDataAsset* ItemData)
{
	CurrentItemUser = CurrentUser;
	TargetCharacter = InTargetCharacter;
	DisputedItem = ItemData;
	
	UpdateConfirmationText();
}

void UItemConfirmationOverlay::ShowOverlay()
{
	SetVisibility(ESlateVisibility::Visible);
	
	// TODO: Implementar bloqueio de interação com resto da UI
	// TODO: Focar no overlay para navegação por teclado
}

void UItemConfirmationOverlay::HideOverlay()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UItemConfirmationOverlay::SetupButtonEvents()
{
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UItemConfirmationOverlay::OnConfirmClicked);
	}
	
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UItemConfirmationOverlay::OnCancelClicked);
	}
}

void UItemConfirmationOverlay::UpdateConfirmationText()
{
	if (!ConfirmationText || !CurrentItemUser || !TargetCharacter || !DisputedItem)
	{
		return;
	}
	
	// Formatar texto com suporte a localização
	FText ConfirmationTextLocalized = FText::Format(
		NSLOCTEXT("Equipment", "ItemConflictMessage", "{0} atualmente utiliza ({1}).\nDeseja equipar este item em {2}?"),
		FText::FromString(CurrentItemUser->GetCharacterName()),
		FText::FromString(DisputedItem->ItemName.ToString()),
		FText::FromString(TargetCharacter->GetCharacterName())
	);
	
	ConfirmationText->SetText(ConfirmationTextLocalized);
}

void UItemConfirmationOverlay::OnConfirmClicked()
{
	ExecuteItemSwap();
	
	// Notificar que a confirmação foi aceita
	OnConfirmationResolved.Broadcast(true, TargetCharacter, CurrentItemUser);
	
	HideOverlay();
}

void UItemConfirmationOverlay::OnCancelClicked()
{
	// Notificar que a confirmação foi cancelada
	OnConfirmationResolved.Broadcast(false, nullptr, nullptr);
	
	HideOverlay();
}

void UItemConfirmationOverlay::ExecuteItemSwap()
{
	if (!CurrentItemUser || !TargetCharacter || !DisputedItem)
	{
		return;
	}
	
	// Obter os componentes de equipamento
	UEquipmentComponent* CurrentUserEquipment = CurrentItemUser->GetEquipmentComponent();
	UEquipmentComponent* TargetEquipment = TargetCharacter->GetEquipmentComponent();
	
	if (!CurrentUserEquipment || !TargetEquipment)
	{
		return;
	}
	
	// Encontrar o slot onde o item está equipado no usuário atual
	EEquipmentSlot ItemSlot = EEquipmentSlot::Weapon; // Usar Weapon como padrão
	UEquippedItem* EquippedItem = nullptr;
	bool bFoundSlot = false;
	
	for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex) // 5 slots: Weapon, Armor, Accessory, Boots, Ring
	{
		EEquipmentSlot EquipmentSlot = static_cast<EEquipmentSlot>(SlotIndex);
		EquippedItem = CurrentUserEquipment->GetEquippedItem(EquipmentSlot);
		
		if (EquippedItem && EquippedItem->GetItemData() == DisputedItem)
		{
			ItemSlot = EquipmentSlot;
			bFoundSlot = true;
			break;
		}
	}
	
	if (!bFoundSlot)
	{
		return;
	}
	
	// Verificar se o personagem alvo tem um item equipado no mesmo slot
	UEquippedItem* TargetSlotItem = TargetEquipment->GetEquippedItem(ItemSlot);
	
	// Guardar referência do item do alvo (se existir) antes de desequipar
	UItemDataAsset* TargetItemData = TargetSlotItem ? TargetSlotItem->GetItemData() : nullptr;
	
	// Desequipar ambos os itens primeiro
	CurrentUserEquipment->UnequipItem(ItemSlot);
	if (TargetSlotItem)
	{
		TargetEquipment->UnequipItem(ItemSlot);
	}
	
	// Equipar o item disputado no personagem alvo
	FInventoryItem ItemToEquip;
	ItemToEquip.ItemData = DisputedItem;
	ItemToEquip.Quantity = 1;
	TargetEquipment->EquipItemFromInventory(ItemToEquip, ItemSlot);
	
	// Se o personagem alvo tinha um item, equipar no usuário atual
	if (TargetItemData)
	{
		FInventoryItem ItemToEquipOnCurrentUser;
		ItemToEquipOnCurrentUser.ItemData = TargetItemData;
		ItemToEquipOnCurrentUser.Quantity = 1;
		CurrentUserEquipment->EquipItemFromInventory(ItemToEquipOnCurrentUser, ItemSlot);
	}
}
