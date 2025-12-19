#include "UI/Equipment/EquipmentOverlayWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"

#include "Inventory/Core/InventorySubsystem.h"
#include "Inventory/Core/InventoryEnums.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquipmentComparisonComponent.h"
#include "Inventory/Equipment/EquippedItem.h"
#include "UI/Equipment/ItemListEntryWidget.h"
#include "UI/Equipment/ItemsDetailsOverlayWidget.h"
#include "UI/Equipment/CharacterSelectorWidget.h"
#include "UI/Equipment/ItemConfirmationOverlay.h"
#include "UI/Equipment/EquipmentStatusWidget.h"
#include "Character/RPGCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Party/PartySubsystem.h"

void UEquipmentOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	InitializeComponents();
	SetupCharacterSelection();
	SetupInitialListState();
	SetupEquipmentStatusWidget(); // Adicionar aqui
}

void UEquipmentOverlayWidget::InitializeComponents()
{
	// Configurar título básico
	if (TitleText)
	{
		TitleText->SetText(FText::FromString(TEXT("Equipment")));
	}

	// Cache dos delegates para evitar recriações
	if (WeaponSlotButton)
	{
		WeaponSlotButton->OnClicked.RemoveAll(this);
		WeaponSlotButton->OnClicked.AddDynamic(this, &UEquipmentOverlayWidget::HandleWeaponSlotButtonClicked);
		WeaponSlotButton->OnHovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleWeaponSlotButtonHovered);
		WeaponSlotButton->OnUnhovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleWeaponSlotButtonUnhovered);
	}

	if (ArmorSlotButton)
	{
		ArmorSlotButton->OnClicked.RemoveAll(this);
		ArmorSlotButton->OnClicked.AddDynamic(this, &UEquipmentOverlayWidget::HandleArmorSlotButtonClicked);
		ArmorSlotButton->OnHovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleArmorSlotButtonHovered);
		ArmorSlotButton->OnUnhovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleArmorSlotButtonUnhovered);
	}

	if (AccessorySlotButton)
	{
		AccessorySlotButton->OnClicked.RemoveAll(this);
		AccessorySlotButton->OnClicked.AddDynamic(this, &UEquipmentOverlayWidget::HandleAccessorySlotButtonClicked);
		AccessorySlotButton->OnHovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleAccessorySlotButtonHovered);
		AccessorySlotButton->OnUnhovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleAccessorySlotButtonUnhovered);
	}

	if (BootsSlotButton)
	{
		BootsSlotButton->OnClicked.RemoveAll(this);
		BootsSlotButton->OnClicked.AddDynamic(this, &UEquipmentOverlayWidget::HandleBootsSlotButtonClicked);
		BootsSlotButton->OnHovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleBootsSlotButtonHovered);
		BootsSlotButton->OnUnhovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleBootsSlotButtonUnhovered);
	}

	if (RingSlotButton)
	{
		RingSlotButton->OnClicked.RemoveAll(this);
		RingSlotButton->OnClicked.AddDynamic(this, &UEquipmentOverlayWidget::HandleRingSlotButtonClicked);
		RingSlotButton->OnHovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleRingSlotButtonHovered);
		RingSlotButton->OnUnhovered.AddDynamic(this, &UEquipmentOverlayWidget::HandleRingSlotButtonUnhovered);
	}

	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveAll(this);
		CloseButton->OnClicked.AddDynamic(this, &UEquipmentOverlayWidget::HandleCloseButtonClicked);
	}
}

// Otimizado: Handlers simplificados
void UEquipmentOverlayWidget::HandleWeaponSlotButtonClicked()
{
	CurrentOpenSlot = EEquipmentSlot::Weapon;
	PopulateItemsList(EInventoryFilterCategory::Weapon, EEquipmentSlot::Weapon);
}

void UEquipmentOverlayWidget::HandleArmorSlotButtonClicked()
{
	CurrentOpenSlot = EEquipmentSlot::Armor;
	PopulateItemsList(EInventoryFilterCategory::Armor, EEquipmentSlot::Armor);
}

void UEquipmentOverlayWidget::HandleAccessorySlotButtonClicked()
{
	CurrentOpenSlot = EEquipmentSlot::Accessory;
	PopulateItemsList(EInventoryFilterCategory::Accessory, EEquipmentSlot::Accessory);
}

void UEquipmentOverlayWidget::HandleBootsSlotButtonClicked()
{
	CurrentOpenSlot = EEquipmentSlot::Boots;
	PopulateItemsList(EInventoryFilterCategory::Boots, EEquipmentSlot::Boots);
}

void UEquipmentOverlayWidget::HandleRingSlotButtonClicked()
{
	CurrentOpenSlot = EEquipmentSlot::Ring;
	PopulateItemsList(EInventoryFilterCategory::Ring, EEquipmentSlot::Ring);
}

// Otimizado: Hover logic centralizada
void UEquipmentOverlayWidget::HandleSlotHovered(EEquipmentSlot SlotType)
{
	// Desativar auto-seleção se estiver ativa
	if (bIsAutoSelectionActive)
	{
		bIsAutoSelectionActive = false;
	}
	
	// Fechar lista se estiver aberta e for slot diferente
	if (IsListVisible() && CurrentOpenSlot != SlotType)
	{
		CloseItemsList();
	}
	
	// IMPORTANTE: Limpar hover de itens APENAS se for slot diferente
	// Isso evita limpar o hover do item atualmente selecionado
	if (CurrentOpenSlot != SlotType)
	{
		ClearItemListHover();
		// Resetar item ativo ao trocar de slot
		ActiveHoverItem = nullptr;
	}

	// Limpar todos os hovers visuais dos slots
	ClearAllSlotHoverVisuals();
	
	// Ativar nova imagem de hover
	ActiveHoverSlot = SlotType;
	SetSlotHoverVisual(SlotType, true);
	
	// Mostrar detalhes do slot
	ShowEquippedItemDetails(SlotType);
}

void UEquipmentOverlayWidget::HandleWeaponSlotButtonHovered()
{
	HandleSlotHovered(EEquipmentSlot::Weapon);
}

void UEquipmentOverlayWidget::HandleArmorSlotButtonHovered()
{
	HandleSlotHovered(EEquipmentSlot::Armor);
}

void UEquipmentOverlayWidget::HandleAccessorySlotButtonHovered()
{
	HandleSlotHovered(EEquipmentSlot::Accessory);
}

void UEquipmentOverlayWidget::HandleBootsSlotButtonHovered()
{
	HandleSlotHovered(EEquipmentSlot::Boots);
}

void UEquipmentOverlayWidget::HandleRingSlotButtonHovered()
{
	HandleSlotHovered(EEquipmentSlot::Ring);
}

// Otimizado: Unhover simplificado
void UEquipmentOverlayWidget::HandleWeaponSlotButtonUnhovered() {}
void UEquipmentOverlayWidget::HandleArmorSlotButtonUnhovered() {}
void UEquipmentOverlayWidget::HandleAccessorySlotButtonUnhovered() {}
void UEquipmentOverlayWidget::HandleBootsSlotButtonUnhovered() {}
void UEquipmentOverlayWidget::HandleRingSlotButtonUnhovered() {}

void UEquipmentOverlayWidget::PopulateItemsList(EInventoryFilterCategory FilterCategory, EEquipmentSlot TargetSlot)
{
	ClearItemsList();

	if (!ItemsScrollBox || !ItemEntryClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemsScrollBox ou ItemEntryClass nao configurados!"));
		return;
	}

	// Cache do subsistema
	UInventorySubsystem* InventorySystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UInventorySubsystem>();
	if (!InventorySystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem nao encontrado!"));
		return;
	}

	TArray<FInventoryItem> CategoryItems = InventorySystem->GetItemsByFilter(FilterCategory);
	
	// Otimização: filtrar itens de uma vez
	TArray<FInventoryItem> FilteredItems;
	if (SelectedCharacter)
	{
		EPlayerClass PlayerClass = SelectedCharacter->GetPlayerClass();
		FilteredItems.Reserve(CategoryItems.Num()); // Pre-alocar memoria
		
		for (const FInventoryItem& Item : CategoryItems)
		{
			if (Item.IsValid() && Item.ItemData && 
				!Item.ItemData->ItemName.IsEmpty() && 
				Item.ItemData->CanClassUse(PlayerClass))
			{
				FilteredItems.Add(Item);
			}
		}
	}
	else
	{
		FilteredItems = CategoryItems;
	}
	
	if (FilteredItems.Num() == 0)
	{
		return;
	}

	// Pre-alocar array de widgets
	ItemEntryWidgets.Reserve(FilteredItems.Num());

	// Criar widgets de forma otimizada
	for (const FInventoryItem& CategoryItem : FilteredItems)
	{
		UItemListEntryWidget* EntryWidget = CreateWidget<UItemListEntryWidget>(this, ItemEntryClass);
		if (!EntryWidget)
		{
			continue;
		}

		EntryWidget->Setup(CategoryItem, TargetSlot, SelectedCharacter);

		// Conectar delegates de uma vez
		EntryWidget->OnItemSelected.AddDynamic(this, &UEquipmentOverlayWidget::OnItemHovered);
		EntryWidget->OnItemEquipped.AddDynamic(this, &UEquipmentOverlayWidget::OnItemEquippedFromList);
		EntryWidget->OnEquipItemRequested.AddDynamic(this, &UEquipmentOverlayWidget::OnEquipItemRequested);

		ItemsScrollBox->AddChild(EntryWidget);
		ItemEntryWidgets.Add(EntryWidget);
	}

	// Tornar elementos visíveis
	if (ItemsListContainer)
	{
		ItemsListContainer->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (ListTxt)
	{
		ListTxt->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (DetailsContainer)
	{
		DetailsContainer->SetVisibility(ESlateVisibility::Visible);
	}
	
	// Auto-hover no primeiro item (mais robusto)
	if (ItemEntryWidgets.Num() > 0)
	{
		bFirstItemAutoHover = true;
		
		// IMPORTANTE: Configurar o primeiro item como ativo
		ActiveHoverItem = ItemEntryWidgets[0];
		ActiveHoverItem->SetHoverVisual(true);
		
		// Aplicar seleção do primeiro item
		ApplyItemSelection(ItemEntryWidgets[0]->GetCurrentItem());
		
		// Log para debug
		UE_LOG(LogTemp, Log, TEXT("ItemListEntryWidget: Auto-hover inicial ativado no primeiro item"));
	}
}

void UEquipmentOverlayWidget::ClearItemsList()
{
	if (!ItemsScrollBox)
	{
		return;
	}

	// Otimizado: cleanup batch
	for (UItemListEntryWidget* EntryWidget : ItemEntryWidgets)
	{
		if (EntryWidget)
		{
			EntryWidget->OnItemSelected.RemoveAll(this);
			EntryWidget->OnItemEquipped.RemoveAll(this);
			EntryWidget->RemoveFromParent();
		}
	}

	ItemsScrollBox->ClearChildren();
	ItemEntryWidgets.Reset(); // Mais eficiente que Empty()
}

void UEquipmentOverlayWidget::SetupEquipmentEvents()
{
	UEquipmentComponent* EquipmentComp = GetSelectedCharacterEquipmentComponent();
	if (!EquipmentComp)
	{
		return;
	}

	EquipmentComp->OnItemEquipped.RemoveAll(this);
	EquipmentComp->OnItemUnequipped.RemoveAll(this);
	EquipmentComp->OnItemEquipped.AddDynamic(this, &UEquipmentOverlayWidget::OnItemEquipped);
	EquipmentComp->OnItemUnequipped.AddDynamic(this, &UEquipmentOverlayWidget::OnItemUnequipped);
}

void UEquipmentOverlayWidget::UpdateAllSlotsDisplay()
{
	// Otimizado: usar array estático para evitar chamadas repetitivas
	static const EEquipmentSlot Slots[] = {
		EEquipmentSlot::Weapon,
		EEquipmentSlot::Armor, 
		EEquipmentSlot::Accessory,
		EEquipmentSlot::Boots,
		EEquipmentSlot::Ring
	};

	for (const EEquipmentSlot& EquipmentSlot : Slots)
	{
		UpdateSlotDisplay(EquipmentSlot);
	}
}

void UEquipmentOverlayWidget::UpdateSlotDisplay(EEquipmentSlot EquipSlot)
{
	// Cache de componentes UI para evitar múltiplas verificações
	struct SlotUIComponents
	{
		UTextBlock* SlotText;
		const TCHAR* SlotName;
	};

	// Tabela de lookup otimizada
	static const TMap<EEquipmentSlot, SlotUIComponents> SlotUIMap = {
		{EEquipmentSlot::Weapon,    {nullptr, TEXT("Arma")}},
		{EEquipmentSlot::Armor,     {nullptr, TEXT("Armadura")}},
		{EEquipmentSlot::Accessory, {nullptr, TEXT("Acessorio")}},
		{EEquipmentSlot::Boots,     {nullptr, TEXT("Bota")}},
		{EEquipmentSlot::Ring,      {nullptr, TEXT("Anel")}}
	};

	UTextBlock* SlotText = nullptr;
	const TCHAR* SlotName = nullptr;

	// Otimizado: switch mais eficiente
	switch (EquipSlot)
	{
		case EEquipmentSlot::Weapon:    SlotText = WeaponSlotText; SlotName = TEXT("Arma"); break;
		case EEquipmentSlot::Armor:     SlotText = ArmorSlotText; SlotName = TEXT("Armadura"); break;
		case EEquipmentSlot::Accessory: SlotText = AccessorySlotText; SlotName = TEXT("Acessorio"); break;
		case EEquipmentSlot::Boots:     SlotText = BootsSlotText; SlotName = TEXT("Bota"); break;
		case EEquipmentSlot::Ring:      SlotText = RingSlotText; SlotName = TEXT("Anel"); break;
		default: return;
	}

	if (!SlotText)
	{
		return;
	}

	UEquipmentComponent* EquipmentComp = GetSelectedCharacterEquipmentComponent();
	if (!EquipmentComp)
	{
		SlotText->SetText(FText::FromString(SlotName));
		return;
	}

	UEquippedItem* EquippedItem = EquipmentComp->GetEquippedItem(EquipSlot);
	if (!EquippedItem || !EquippedItem->GetItemData())
	{
		SlotText->SetText(FText::FromString(SlotName));
		return;
	}

	SlotText->SetText(EquippedItem->GetItemData()->ItemName);
	SetupSlotIcon(EquipSlot);
}

void UEquipmentOverlayWidget::UpdateAllEquipmentDependentWidgets()
{
	// Atualizar displays de todos os slots
	UpdateAllSlotsDisplay();
	
	// Atualizar ícones dos slots (ícones visuais dos itens equipados)
	SetupSlotIcons();
	
	// Atualizar status de equipamento (dano total)
	if (EquipmentStatusWidget)
	{
		EquipmentStatusWidget->UpdateDisplay();
		UE_LOG(LogTemp, Log, TEXT("EquipmentOverlay: EquipmentStatusWidget atualizado após mudança de equipamento"));
	}
	
	// Atualizar ícones de classe
	UpdateItemListIcons();
	
	// Outros widgets que dependem de equipamentos podem ser adicionados aqui
	// Exemplo: Widget de stats, Widget de resistências, etc.
	
	UE_LOG(LogTemp, Log, TEXT("EquipmentOverlay: Todos os widgets dependentes de equipamento foram atualizados"));
}

void UEquipmentOverlayWidget::OnItemEquipped(EEquipmentSlot EquipSlot, UEquippedItem* EquippedItem)
{
	// Atualizar todos os widgets que dependem de equipamentos
	UpdateAllEquipmentDependentWidgets();
	
	// Ativar hover no slot equipado
	ActivateSlotHover(EquipSlot);
}

void UEquipmentOverlayWidget::OnItemUnequipped(EEquipmentSlot EquipSlot, const FInventoryItem& UnequippedItem)
{
	// Atualizar todos os widgets que dependem de equipamentos
	UpdateAllEquipmentDependentWidgets();
}

void UEquipmentOverlayWidget::OnItemEquippedFromList(const FInventoryItem& EquippedItem)
{
	// Delegate para OnItemEquipped fazer o cleanup
}

void UEquipmentOverlayWidget::OnItemHovered(const FInventoryItem& HoveredItem)
{
	// Desativar auto-seleção se estiver ativa
	if (bIsAutoSelectionActive)
	{
		bIsAutoSelectionActive = false;
	}
	
	// Desativar auto-hover inicial se estiver ativo
	if (bFirstItemAutoHover)
	{
		bFirstItemAutoHover = false;
	}
	
	// IMPORTANTE: Atualizar hover visual de forma mais robusta
	// Primeiro, desativar o item anterior se for diferente
	if (ActiveHoverItem && ActiveHoverItem->GetCurrentItem().ItemData != HoveredItem.ItemData)
	{
		ActiveHoverItem->SetHoverVisual(false);
		ActiveHoverItem = nullptr;
	}
	
	// Encontrar e ativar o novo item
	for (UItemListEntryWidget* EntryWidget : ItemEntryWidgets)
	{
		if (EntryWidget && EntryWidget->GetCurrentItem().ItemData == HoveredItem.ItemData)
		{
			// Ativar hover visual
			EntryWidget->SetHoverVisual(true);
			ActiveHoverItem = EntryWidget;
			
			// Log para debug
			UE_LOG(LogTemp, VeryVerbose, TEXT("ItemListEntryWidget: Hover ativado em %s"), 
				*HoveredItem.ItemData->ItemName.ToString());
			break;
		}
	}
	
	// SIMULAR EQUIPAMENTO para comparação de stats
	SimulateEquipmentForComparison(HoveredItem);
	
	// Aplicar seleção do item (mostrar detalhes)
	ApplyItemSelection(HoveredItem);
}

void UEquipmentOverlayWidget::ApplyItemSelection(const FInventoryItem& SelectedItem)
{
	ClearEquipmentDetailsWidget();
	
	if (EquipmentDetailsClass && DetailsContainer)
	{
		EquipmentDetailsWidget = CreateWidget<UItemsDetailsOverlayWidget>(this, EquipmentDetailsClass);
		if (EquipmentDetailsWidget)
		{
			DetailsContainer->SetContent(EquipmentDetailsWidget);
			EquipmentDetailsWidget->Setup(SelectedItem);
			
			if (DetailsContainer)
			{
				DetailsContainer->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}

void UEquipmentOverlayWidget::ShowEquippedItemDetails(EEquipmentSlot EquipSlot)
{
	UEquipmentComponent* EquipmentComp = GetSelectedCharacterEquipmentComponent();
	if (!EquipmentComp)
	{
		return;
	}

	UEquippedItem* EquippedItem = EquipmentComp->GetEquippedItem(EquipSlot);
	if (!EquippedItem || !EquippedItem->GetItemData())
	{
		// Slot vazio
		if (DetailsContainer)
		{
			DetailsContainer->SetVisibility(ESlateVisibility::Collapsed);
		}
		
		ClearEquipmentDetailsWidget();
		
		if (ItemsListContainer && ItemsListContainer->GetVisibility() == ESlateVisibility::Visible)
		{
			ItemsListContainer->SetVisibility(ESlateVisibility::Collapsed);
		}
		
		if (ListTxt)
		{
			ListTxt->SetVisibility(ESlateVisibility::Collapsed);
		}
		
		ClearItemListHover();
		return;
	}

	FInventoryItem InventoryItem = EquippedItem->GetSourceItem();
	ApplyItemSelection(InventoryItem);
}

FString UEquipmentOverlayWidget::GetSlotDisplayName(EEquipmentSlot EquipSlot) const
{
	// Otimizado: usar tabela estática
	static const TMap<EEquipmentSlot, FString> SlotNames = {
		{EEquipmentSlot::Weapon,    TEXT("Arma")},
		{EEquipmentSlot::Armor,     TEXT("Armadura")},
		{EEquipmentSlot::Accessory, TEXT("Acessorio")},
		{EEquipmentSlot::Boots,     TEXT("Bota")},
		{EEquipmentSlot::Ring,      TEXT("Anel")}
	};

	const FString* FoundName = SlotNames.Find(EquipSlot);
	return FoundName ? *FoundName : TEXT("Desconhecido");
}

void UEquipmentOverlayWidget::SetupSlotIcons()
{
	static const EEquipmentSlot Slots[] = {
		EEquipmentSlot::Weapon, EEquipmentSlot::Armor, EEquipmentSlot::Accessory,
		EEquipmentSlot::Boots, EEquipmentSlot::Ring
	};

	for (const EEquipmentSlot& EquipmentSlot : Slots)
	{
		SetupSlotIcon(EquipmentSlot);
	}
}

void UEquipmentOverlayWidget::SetupSlotIcon(EEquipmentSlot EquipSlot)
{
	UImage* SlotIcon = nullptr;

	switch (EquipSlot)
	{
		case EEquipmentSlot::Weapon:    SlotIcon = WeaponSlotIcon; break;
		case EEquipmentSlot::Armor:     SlotIcon = ArmorSlotIcon; break;
		case EEquipmentSlot::Accessory: SlotIcon = AccessorySlotIcon; break;
		case EEquipmentSlot::Boots:     SlotIcon = BootsSlotIcon; break;
		case EEquipmentSlot::Ring:      SlotIcon = RingSlotIcon; break;
		default: return;
	}

	if (!SlotIcon)
	{
		return;
	}

	UEquipmentComponent* EquipmentComp = GetSelectedCharacterEquipmentComponent();
	if (!EquipmentComp)
	{
		SetDefaultSlotIcon(SlotIcon, EquipSlot);
		return;
	}

	UEquippedItem* EquippedItem = EquipmentComp->GetEquippedItem(EquipSlot);
	if (!EquippedItem || !EquippedItem->GetItemData())
	{
		SetDefaultSlotIcon(SlotIcon, EquipSlot);
		return;
	}

	// Cache da textura para evitar múltiplos LoadSynchronous
	UTexture2D* IconTexture = EquippedItem->GetItemData()->TypeIcon.LoadSynchronous();
	if (IconTexture)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(IconTexture);
		
		Brush.ImageSize = FVector2D(30.0f, 30.0f);
		
		SlotIcon->SetBrush(Brush);
		SlotIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetDefaultSlotIcon(SlotIcon, EquipSlot);
	}
}

void UEquipmentOverlayWidget::SetDefaultSlotIcon(UImage* SlotIcon, EEquipmentSlot EquipSlot)
{
	SlotIcon->SetVisibility(ESlateVisibility::Collapsed);
}

void UEquipmentOverlayWidget::SetSlotHoverVisual(EEquipmentSlot EquipSlot, bool bHovered)
{
	UImage* HoverImage = nullptr;

	switch (EquipSlot)
	{
		case EEquipmentSlot::Weapon:    HoverImage = WeaponSlotHoverImage; break;
		case EEquipmentSlot::Armor:     HoverImage = ArmorSlotHoverImage; break;
		case EEquipmentSlot::Accessory: HoverImage = AccessorySlotHoverImage; break;
		case EEquipmentSlot::Boots:     HoverImage = BootsSlotHoverImage; break;
		case EEquipmentSlot::Ring:      HoverImage = RingSlotHoverImage; break;
		default: return;
	}

	if (HoverImage)
	{
		HoverImage->SetVisibility(bHovered ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UEquipmentOverlayWidget::ClearAllSlotHoverVisuals()
{
	static const EEquipmentSlot Slots[] = {
		EEquipmentSlot::Weapon, EEquipmentSlot::Armor, EEquipmentSlot::Accessory,
		EEquipmentSlot::Boots, EEquipmentSlot::Ring
	};

	for (const EEquipmentSlot& EquipmentSlot : Slots)
	{
		SetSlotHoverVisual(EquipmentSlot, false);
	}
}

void UEquipmentOverlayWidget::CloseItemsList(bool bPreserveSlotHover)
{
	if (ItemsListContainer)
	{
		ItemsListContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (ListTxt)
	{
		ListTxt->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (!bPreserveSlotHover && DetailsContainer)
	{
		DetailsContainer->SetVisibility(ESlateVisibility::Collapsed);
		ClearEquipmentDetailsWidget();
	}

	if (!bPreserveSlotHover)
	{
		SetSlotHoverVisual(ActiveHoverSlot, false);
		ActiveHoverSlot = EEquipmentSlot::Weapon;
		ClearAllSlotHoverVisuals();
	}
	
	bFirstItemAutoHover = false;
	ActiveHoverItem = nullptr;
}

bool UEquipmentOverlayWidget::IsListVisible() const
{
	return ItemsListContainer && ItemsListContainer->GetVisibility() == ESlateVisibility::Visible;
}

void UEquipmentOverlayWidget::ClearItemListHover()
{
	// IMPORTANTE: Limpar hover de todos os itens e resetar estado
	for (UItemListEntryWidget* EntryWidget : ItemEntryWidgets)
	{
		if (EntryWidget)
		{
			EntryWidget->ResetHoverState();
		}
	}
	
	// Resetar item ativo
	ActiveHoverItem = nullptr;
	
	// LIMPAR SIMULAÇÃO quando sair do hover
	ClearEquipmentSimulation();
	
	// Log para debug
	UE_LOG(LogTemp, VeryVerbose, TEXT("ItemListEntryWidget: Hover de todos os itens limpo"));
}

void UEquipmentOverlayWidget::HandleCloseButtonClicked()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UEquipmentOverlayWidget::AutoSelectWeaponSlot()
{
	bIsAutoSelectionActive = true;
	ActiveHoverSlot = EEquipmentSlot::Weapon;
	SetSlotHoverVisual(EEquipmentSlot::Weapon, true);
	ShowEquippedItemDetails(EEquipmentSlot::Weapon);
}

void UEquipmentOverlayWidget::ActivateSlotHover(EEquipmentSlot EquipSlot)
{
	ClearAllSlotHoverVisuals();
	ClearItemListHover();
	
	if (IsListVisible())
	{
		CloseItemsList(false);
	}
	
	ActiveHoverSlot = EquipSlot;
	SetSlotHoverVisual(EquipSlot, true);
	ShowEquippedItemDetails(EquipSlot);
}

// ===== CHARACTER SELECTION - OTIMIZADA =====

void UEquipmentOverlayWidget::SetupCharacterSelection()
{
	ClearCharacterSelectors();
	PopulateCharacterSelectors();
}

void UEquipmentOverlayWidget::PopulateCharacterSelectors()
{
	if (!CharacterSelectorClass)
	{
		return;
	}
	
	UPartySubsystem* PartySubsystem = nullptr;
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>();
	}
	
	if (!PartySubsystem)
	{
		return;
	}
	
	TArray<ARPGCharacter*> PartyMembers = PartySubsystem->GetAllPartyMembers();
	if (PartyMembers.Num() == 0)
	{
		return;
	}
	
	TArray<ARPGCharacter*> OrderedMembers = OrderPartyMembers(PartyMembers);
	
	// Pre-alocar array
	CharacterSelectorWidgets.Reserve(OrderedMembers.Num());
	
	for (ARPGCharacter* PartyMember : OrderedMembers)
	{
		if (!PartyMember)
		{
			continue;
		}
		
		UCharacterSelectorWidget* MemberSelector = CreateWidget<UCharacterSelectorWidget>(this, CharacterSelectorClass);
		if (!MemberSelector)
		{
			continue;
		}
		
		FString CharacterDisplayName = GetCharacterDisplayName(PartyMember);
		MemberSelector->Setup(PartyMember, CharacterDisplayName);
		MemberSelector->OnCharacterSelected.AddDynamic(this, &UEquipmentOverlayWidget::OnCharacterSelected);
		
		CharacterSelectorWidgets.Add(MemberSelector);
		
		if (CharacterSelectorsContainer)
		{
			CharacterSelectorsContainer->AddChild(MemberSelector);
		}
	}
	
	if (OrderedMembers.Num() > 0)
	{
		UpdateSelectedCharacter(OrderedMembers[0]);
	}
}

TArray<ARPGCharacter*> UEquipmentOverlayWidget::OrderPartyMembers(const TArray<ARPGCharacter*>& PartyMembers)
{
	TArray<ARPGCharacter*> OrderedMembers;
	
	// Cache da ordem desejada
	static const TArray<FString> DesiredOrder = { TEXT("Yumi"), TEXT("Axix"), TEXT("Yoshino"), TEXT("Shimako") };
	
	OrderedMembers.Reserve(PartyMembers.Num());
	
	for (const FString& DesiredName : DesiredOrder)
	{
		for (ARPGCharacter* PartyMember : PartyMembers)
		{
			if (PartyMember && GetCharacterDisplayName(PartyMember) == DesiredName)
			{
				OrderedMembers.Add(PartyMember);
				break;
			}
		}
	}
	
	// Adicionar personagens restantes
	for (ARPGCharacter* PartyMember : PartyMembers)
	{
		if (PartyMember && !OrderedMembers.Contains(PartyMember))
		{
			OrderedMembers.Add(PartyMember);
		}
	}
	
	return OrderedMembers;
}

void UEquipmentOverlayWidget::ClearCharacterSelectors()
{
	for (UCharacterSelectorWidget* Selector : CharacterSelectorWidgets)
	{
		if (Selector)
		{
			Selector->RemoveFromParent();
		}
	}
	
	CharacterSelectorWidgets.Reset();
}

void UEquipmentOverlayWidget::UpdateAllSelectorsVisualState()
{
	for (UCharacterSelectorWidget* Selector : CharacterSelectorWidgets)
	{
		if (Selector)
		{
			bool bIsSelected = (Selector->GetCurrentCharacter() == SelectedCharacter);
			Selector->SetSelectedState(bIsSelected);
		}
	}
}

void UEquipmentOverlayWidget::ClearEquipmentDetailsWidget()
{
	if (EquipmentDetailsWidget)
	{
		EquipmentDetailsWidget->RemoveFromParent();
		EquipmentDetailsWidget = nullptr;
	}
}

void UEquipmentOverlayWidget::SetupEquipmentStatusWidget()
{
	ClearEquipmentStatusWidget();
	
	if (EquipmentStatusClass && EquipmentStatusContainer)
	{
		EquipmentStatusWidget = CreateWidget<UEquipmentStatusWidget>(this, EquipmentStatusClass);
		if (EquipmentStatusWidget)
		{
			EquipmentStatusContainer->SetContent(EquipmentStatusWidget);
			
			// Configurar o personagem selecionado
			if (SelectedCharacter)
			{
				EquipmentStatusWidget->SetTargetCharacter(SelectedCharacter);
			}
			
			// Tornar o container visível
			EquipmentStatusContainer->SetVisibility(ESlateVisibility::Visible);
			
			UE_LOG(LogTemp, Log, TEXT("EquipmentOverlay: EquipmentStatusWidget criado e configurado"));
		}
	}
}

void UEquipmentOverlayWidget::ClearEquipmentStatusWidget()
{
	if (EquipmentStatusWidget)
	{
		EquipmentStatusWidget->RemoveFromParent();
		EquipmentStatusWidget = nullptr;
	}
}

void UEquipmentOverlayWidget::SetupInitialListState()
{
	if (ItemsListContainer)
	{
		ItemsListContainer->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (ListTxt)
	{
		ListTxt->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEquipmentOverlayWidget::OnCharacterSelected(ARPGCharacter* SelectedChar)
{
	if (!SelectedChar || SelectedChar == SelectedCharacter)
	{
		return;
	}
	
	UpdateSelectedCharacter(SelectedChar);
	UpdateAllSelectorsVisualState();
}

void UEquipmentOverlayWidget::UpdateSelectedCharacter(ARPGCharacter* NewSelectedCharacter)
{
	SelectedCharacter = NewSelectedCharacter;
	
	if (!SelectedCharacter)
	{
		return;
	}
	
	// Atualizar nome do personagem
	if (CharacterName)
	{
		FString CharacterDisplayName = GetCharacterDisplayName(SelectedCharacter);
		CharacterName->SetText(FText::FromString(CharacterDisplayName));
	}
	
	// Reconfigurar sistema de forma otimizada
	SetupEquipmentEvents();
	UpdateAllEquipmentDependentWidgets(); // Usar a nova função unificada
	
	// Atualizar o EquipmentStatusWidget com o novo personagem
	if (EquipmentStatusWidget)
	{
		EquipmentStatusWidget->SetTargetCharacter(SelectedCharacter);
	}
	
	if (IsListVisible())
	{
		UpdateItemListIcons();
		CloseItemsList();
	}
	
	ClearEquipmentDetailsWidget();
	ClearAllSlotHoverVisuals();
	ClearItemListHover();
	
	ActiveHoverSlot = EEquipmentSlot::Weapon;
	AutoSelectWeaponSlot();
	UpdateAllSelectorsVisualState();
}

UEquipmentComponent* UEquipmentOverlayWidget::GetSelectedCharacterEquipmentComponent() const
{
	if (!SelectedCharacter)
	{
		return nullptr;
	}

	return SelectedCharacter->GetComponentByClass<UEquipmentComponent>();
}

void UEquipmentOverlayWidget::OnEquipItemRequested(const FInventoryItem& ItemToEquip, EEquipmentSlot TargetSlot)
{
	if (!ItemToEquip.ItemData)
	{
		return;
	}

	UEquipmentComponent* EquipmentComp = GetSelectedCharacterEquipmentComponent();
	if (!EquipmentComp)
	{
		return;
	}

	ARPGCharacter* ConflictCharacter = nullptr;
	if (CheckItemConflict(ItemToEquip, TargetSlot, ConflictCharacter))
	{
		ShowItemConfirmationOverlay(ConflictCharacter, ItemToEquip, TargetSlot);
		return;
	}

	bool bSuccess = EquipmentComp->EquipItemFromInventory(ItemToEquip, TargetSlot);
	if (!bSuccess)
	{
		FString ItemName = ItemToEquip.ItemData->ItemName.ToString();
		FString SlotName = GetSlotDisplayName(TargetSlot);
		UE_LOG(LogTemp, Error, TEXT("Falha ao equipar item '%s' no slot %s para personagem: %s!"), 
			*ItemName, *SlotName, *GetCharacterDisplayName(SelectedCharacter));
	}
}

void UEquipmentOverlayWidget::UpdateItemListIcons()
{
	for (UItemListEntryWidget* EntryWidget : ItemEntryWidgets)
	{
		if (EntryWidget)
		{
			EntryWidget->UpdateCharacterClassIcon();
		}
	}
}

FString UEquipmentOverlayWidget::GetCharacterDisplayName(ARPGCharacter* Character) const
{
	if (!Character)
	{
		return TEXT("Unknown");
	}
	
	return Character->GetCharacterName();
}

// ===== CONFLICT SYSTEM - OTIMIZADA =====

bool UEquipmentOverlayWidget::CheckItemConflict(const FInventoryItem& ItemToEquip, EEquipmentSlot TargetSlot, ARPGCharacter*& OutConflictCharacter)
{
	// Early exit para itens com restrição de classe
	if (ItemToEquip.ItemData->AllowedClasses.Num() > 0)
	{
		return false;
	}

	UPartySubsystem* PartySubsystem = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UPartySubsystem>();
	if (!PartySubsystem)
	{
		return false;
	}

	TArray<ARPGCharacter*> PartyMembers = PartySubsystem->GetAllPartyMembers();
	
	// Cache dos slots para verificação otimizada
	static const EEquipmentSlot SlotsToCheck[] = {
		EEquipmentSlot::Weapon, EEquipmentSlot::Armor, EEquipmentSlot::Accessory,
		EEquipmentSlot::Boots, EEquipmentSlot::Ring
	};

	for (ARPGCharacter* PartyMember : PartyMembers)
	{
		if (PartyMember == SelectedCharacter)
		{
			continue;
		}

		UEquipmentComponent* MemberEquipment = PartyMember->GetEquipmentComponent();
		if (!MemberEquipment)
		{
			continue;
		}

		// Verificação otimizada usando array estático
		for (const EEquipmentSlot& EquipmentSlot : SlotsToCheck)
		{
			UEquippedItem* EquippedItem = MemberEquipment->GetEquippedItem(EquipmentSlot);
			
			if (EquippedItem && EquippedItem->GetItemData() == ItemToEquip.ItemData)
			{
				OutConflictCharacter = PartyMember;
				return true;
			}
		}
	}

	return false;
}

void UEquipmentOverlayWidget::ShowItemConfirmationOverlay(ARPGCharacter* ConflictCharacter, const FInventoryItem& ItemToEquip, EEquipmentSlot TargetSlot)
{
	if (!ItemConfirmationWidget)
	{
		if (ItemConfirmationClass && ConfirmationContainer)
		{
			ItemConfirmationWidget = CreateWidget<UItemConfirmationOverlay>(this, ItemConfirmationClass);
			if (ItemConfirmationWidget)
			{
				ConfirmationContainer->SetContent(ItemConfirmationWidget);
				ItemConfirmationWidget->OnConfirmationResolved.AddDynamic(this, &UEquipmentOverlayWidget::OnItemConfirmationResolved);
			}
		}
	}

	if (ItemConfirmationWidget)
	{
		ItemConfirmationWidget->SetupConfirmation(ConflictCharacter, SelectedCharacter, ItemToEquip.ItemData);
		ItemConfirmationWidget->ShowOverlay();
	}
}

void UEquipmentOverlayWidget::OnItemConfirmationResolved(bool bConfirmed, ARPGCharacter* NewUser, ARPGCharacter* PreviousUser)
{
	if (bConfirmed)
	{
		UpdateAllSlotsDisplay();
		UpdateItemListIcons();
	}

	if (ItemConfirmationWidget)
	{
		ItemConfirmationWidget->HideOverlay();
	}
}

void UEquipmentOverlayWidget::SimulateEquipmentForComparison(const FInventoryItem& ItemToSimulate)
{
	if (!SelectedCharacter || !ItemToSimulate.IsValid())
	{
		return;
	}
	
	// Obter o componente de comparação
	UEquipmentComparisonComponent* CompComponent = SelectedCharacter->GetEquipmentComparisonComponent();
	if (!CompComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentOverlay: Character não tem EquipmentComparisonComponent"));
		return;
	}
	
	// Simular o equipamento no slot atual
	CompComponent->SimulateEquipment(CurrentOpenSlot, ItemToSimulate);
	
	// Atualizar o widget de status para mostrar os valores simulados
	if (EquipmentStatusWidget)
	{
		EquipmentStatusWidget->UpdateDisplay();
	}
	
	UE_LOG(LogTemp, Log, TEXT("EquipmentOverlay: Simulação iniciada para item %s no slot %s"), 
		*ItemToSimulate.ItemData->ItemName.ToString(), *UEnum::GetValueAsString(CurrentOpenSlot));
}

void UEquipmentOverlayWidget::ClearEquipmentSimulation()
{
	if (!SelectedCharacter)
	{
		return;
	}
	
	// Obter o componente de comparação
	UEquipmentComparisonComponent* CompComponent = SelectedCharacter->GetEquipmentComparisonComponent();
	if (!CompComponent)
	{
		return;
	}
	
	// Limpar simulação do slot atual
	CompComponent->ClearSimulation(CurrentOpenSlot);
	
	// Atualizar o widget de status para mostrar os valores reais
	if (EquipmentStatusWidget)
	{
		EquipmentStatusWidget->UpdateDisplay();
	}
	
	UE_LOG(LogTemp, Log, TEXT("EquipmentOverlay: Simulação limpa para slot %s"), 
		*UEnum::GetValueAsString(CurrentOpenSlot));
}