#include "UI/Equipment/ItemListEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquippedItem.h"
#include "Character/RPGCharacter.h"
#include "Character/PlayerClassInfo.h"
#include "Game/RPGGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Party/PartySubsystem.h"

void UItemListEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Configurar botão de equipar e hover
	if (EquipButton)
	{
		EquipButton->OnClicked.RemoveAll(this);
		EquipButton->OnClicked.AddDynamic(this, &UItemListEntryWidget::HandleEquipButtonClicked);
		EquipButton->OnHovered.AddDynamic(this, &UItemListEntryWidget::HandleButtonHovered);
		EquipButton->OnUnhovered.AddDynamic(this, &UItemListEntryWidget::HandleButtonUnhovered);
	}
	
	// Iniciar hover oculto
	if (HoverOverlayImage)
	{
		HoverOverlayImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UItemListEntryWidget::Setup(const FInventoryItem& EquipmentItem, EEquipmentSlot InTargetSlot, ARPGCharacter* InSelectedCharacter)
{
	CurrentItem = EquipmentItem;
	TargetSlot = InTargetSlot;
	SelectedCharacter = InSelectedCharacter;
	
	UpdateDisplay();
	UpdateCharacterClassIcon();
	
	// Garantir que o hover está resetado
	ResetHoverState();
}

void UItemListEntryWidget::Clear()
{
	CurrentItem = FInventoryItem();
	TargetSlot = EEquipmentSlot::Weapon;
	SelectedCharacter = nullptr; // BUG FIX: Limpar referência do personagem
	UpdateDisplay();
	ResetHoverState(); // BUG FIX: Resetar hover ao limpar
	
	// BUG FIX: Ocultar ícone da classe ao limpar
	if (CharacterClassIcon)
	{
		CharacterClassIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UItemListEntryWidget::UpdateDisplay()
{
	if (!ItemNameText) return;

	if (!HasItem())
	{
		ItemNameText->SetText(FText::FromString(TEXT("Item Inválido")));
		
		// BUG FIX: Desabilitar botão se não há item válido
		if (EquipButton)
		{
			EquipButton->SetIsEnabled(false);
		}
		return;
	}

	ItemNameText->SetText(FText::FromString(CurrentItem.ItemData->ItemName.ToString()));
	
	// BUG FIX: Habilitar botão se há item válido
	if (EquipButton)
	{
		EquipButton->SetIsEnabled(true);
	}
}

void UItemListEntryWidget::HandleEquipButtonClicked()
{
	// BUG FIX: Validações adicionais antes de equipar
	if (!HasItem() || !SelectedCharacter) 
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemListEntryWidget: Tentativa de equipar item inválido ou personagem nulo"));
		return;
	}

	// BUG FIX: Verificar se o personagem tem componente de equipamento
	UEquipmentComponent* EquipmentComp = SelectedCharacter->GetEquipmentComponent();
	if (!EquipmentComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemListEntryWidget: Personagem não possui componente de equipamento"));
		return;
	}

	// Disparar delegate para equipar no personagem selecionado
	OnEquipItemRequested.Broadcast(CurrentItem, TargetSlot);
}

void UItemListEntryWidget::HandleButtonHovered()
{
	if (!HasItem()) return;

	SetHoverVisual(true);
	OnItemSelected.Broadcast(CurrentItem);
}

void UItemListEntryWidget::HandleButtonUnhovered()
{
	// IMPORTANTE: Não resetar hover automaticamente ao sair do botão
	// O sistema de hover ativo do EquipmentOverlayWidget deve controlar isso
	// Isso evita flickering e mantém o hover consistente
	
	// Log para debug
	UE_LOG(LogTemp, VeryVerbose, TEXT("ItemListEntryWidget: Unhover detectado - mantendo estado de hover"));
}

FString UItemListEntryWidget::GetSlotDisplayName() const
{
	switch (TargetSlot)
	{
		case EEquipmentSlot::Weapon:    return TEXT("Arma");
		case EEquipmentSlot::Armor:     return TEXT("Armadura");
		case EEquipmentSlot::Accessory: return TEXT("Acessório");
		case EEquipmentSlot::Boots:     return TEXT("Bota");
		case EEquipmentSlot::Ring:      return TEXT("Anel");
		default: return TEXT("Desconhecido");
	}
}

void UItemListEntryWidget::SetHoverVisual(bool bHovered)
{
	if (HoverOverlayImage)
	{
		HoverOverlayImage->SetVisibility(bHovered ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UItemListEntryWidget::ResetHoverState()
{
	SetHoverVisual(false);
}

void UItemListEntryWidget::UpdateCharacterClassIcon()
{
	// BUG FIX: Validação mais robusta no início
	if (!CharacterClassIcon)
	{
		return;
	}

	if (!SelectedCharacter || !HasItem())
	{
		CharacterClassIcon->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	ARPGCharacter* ConflictCharacter = nullptr;
	bool bHasConflict = false;
	
	// Verifica se item compartilhado já está em uso por outro personagem
	if (CurrentItem.ItemData->AllowedClasses.Num() == 0)
	{
		// BUG FIX: Validações adicionais para evitar crashes
		UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
		if (!GameInstance)
		{
			CharacterClassIcon->SetVisibility(ESlateVisibility::Collapsed);
			return;
		}

		UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>();
		if (PartySubsystem)
		{
			TArray<ARPGCharacter*> PartyMembers = PartySubsystem->GetAllPartyMembers();
			for (ARPGCharacter* PartyMember : PartyMembers)
			{
				if (PartyMember == SelectedCharacter || !IsValid(PartyMember)) continue;

				UEquipmentComponent* MemberEquipment = PartyMember->GetEquipmentComponent();
				if (!MemberEquipment) continue;

				// BUG FIX: Usar array estático para os slots
				static const EEquipmentSlot Slots[] = {
					EEquipmentSlot::Weapon, EEquipmentSlot::Armor, EEquipmentSlot::Accessory,
					EEquipmentSlot::Boots, EEquipmentSlot::Ring
				};
				
				for (const EEquipmentSlot& EquipmentSlot : Slots)
				{
					UEquippedItem* EquippedItem = MemberEquipment->GetEquippedItem(EquipmentSlot);
					
					if (EquippedItem && IsValid(EquippedItem->GetItemData()) && 
						EquippedItem->GetItemData() == CurrentItem.ItemData)
					{
						ConflictCharacter = PartyMember;
						bHasConflict = true;
						break;
					}
				}
				if (bHasConflict) break;
			}
		}
	}

	// Verificar se o item está equipado no personagem selecionado
	UEquipmentComponent* EquipmentComp = SelectedCharacter->GetEquipmentComponent();
	if (!EquipmentComp)
	{
		CharacterClassIcon->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	UEquippedItem* EquippedItem = EquipmentComp->GetEquippedItem(TargetSlot);
	bool bIsEquipped = (EquippedItem && IsValid(EquippedItem->GetItemData()) && 
						EquippedItem->GetItemData() == CurrentItem.ItemData);

	// Definir personagem a mostrar no ícone
	ARPGCharacter* CharacterToShow = nullptr;
	if (bIsEquipped) 
	{
		CharacterToShow = SelectedCharacter;
	}
	else if (bHasConflict) 
	{
		CharacterToShow = ConflictCharacter;
	}
	else
	{
		CharacterClassIcon->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// BUG FIX: Validar se CharacterToShow é válido
	if (!IsValid(CharacterToShow))
	{
		CharacterClassIcon->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	EPlayerClass PlayerClass = CharacterToShow->GetPlayerClass();
	ARPGGameModeBase* GameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!GameMode || !GameMode->PlayerClassInfo)
	{
		CharacterClassIcon->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// Mostrar ícone da classe
	FPlayerClassData ClassData = GameMode->PlayerClassInfo->GetPlayerClassInfo(PlayerClass);
	if (ClassData.ClassIcon && IsValid(ClassData.ClassIcon))
	{
		CharacterClassIcon->SetBrushFromTexture(ClassData.ClassIcon);
		CharacterClassIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		CharacterClassIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	// BUG FIX: Log para debug se necessário
	UE_LOG(LogTemp, VeryVerbose, TEXT("ItemListEntryWidget: Ícone atualizado - Equipado: %s, Conflito: %s"), 
		   bIsEquipped ? TEXT("Sim") : TEXT("Não"), 
		   bHasConflict ? TEXT("Sim") : TEXT("Não"));
}

