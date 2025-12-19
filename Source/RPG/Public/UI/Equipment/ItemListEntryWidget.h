#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/RPGUserWidget.h"
#include "Inventory/Core/InventoryTypes.h"
#include "Inventory/Core/InventoryEnums.h"
#include "Inventory/Equipment/EquipmentComponent.h"
#include "ItemListEntryWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;
class UEquipmentComponent;

/**
 * Widget de entrada genérico para qualquer item de equipamento
 * Funciona com qualquer categoria: Weapon, Armor, Accessory, Boots, Ring
 */
UCLASS()
class RPG_API UItemListEntryWidget : public URPGUserWidget
{
	GENERATED_BODY()

public:
	/** Configurar o widget com dados do item, slot de destino e personagem selecionado */
	UFUNCTION(BlueprintCallable, Category = "Item Entry")
	void Setup(const FInventoryItem& EquipmentItem, EEquipmentSlot InTargetSlot, ARPGCharacter* InSelectedCharacter);

	/** Limpar o widget */
	UFUNCTION(BlueprintCallable, Category = "Item Entry")
	void Clear();

	/** Verificar se tem item válido */
	UFUNCTION(BlueprintPure, Category = "Item Entry")
	bool HasItem() const { return CurrentItem.ItemData != nullptr; }

	/** Obter item atual */
	UFUNCTION(BlueprintPure, Category = "Item Entry")
	const FInventoryItem& GetCurrentItem() const { return CurrentItem; }

	/** Obter slot de destino */
	UFUNCTION(BlueprintPure, Category = "Item Entry")
	EEquipmentSlot GetTargetSlot() const { return TargetSlot; }

	/** Delegate para quando item é selecionado/hovered */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemSelected, const FInventoryItem&, SelectedItem);
	UPROPERTY(BlueprintAssignable, Category = "Item Entry")
	FOnItemSelected OnItemSelected;

	/** Delegate para quando item é equipado com sucesso */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemEquipped, const FInventoryItem&, EquippedItem);
	UPROPERTY(BlueprintAssignable, Category = "Item Entry")
	FOnItemEquipped OnItemEquipped;

	/** Delegate para equipar item no personagem selecionado (NOVO) */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipItemRequested, const FInventoryItem&, ItemToEquip, EEquipmentSlot, TargetSlot);
	UPROPERTY(BlueprintAssignable, Category = "Item Entry")
	FOnEquipItemRequested OnEquipItemRequested;

	/** Configurar imagem de hover como visível/oculta */
	UFUNCTION(BlueprintCallable, Category = "Item Entry")
	void SetHoverVisual(bool bHovered);

	/** Resetar estado de hover */
	UFUNCTION(BlueprintCallable, Category = "Item Entry")
	void ResetHoverState();

	/** Atualizar ícone da classe do personagem (público para EquipmentOverlayWidget) */
	UFUNCTION(BlueprintCallable, Category = "Item Entry")
	void UpdateCharacterClassIcon();

protected:
	virtual void NativeConstruct() override;

	/** Texto do nome do item */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText = nullptr;

	/** Botão para equipar o item */
	UPROPERTY(meta = (BindWidget))
	UButton* EquipButton = nullptr;

	/** Imagem de overlay para efeito de hover */
	UPROPERTY(meta = (BindWidget))
	UImage* HoverOverlayImage = nullptr;

	/** Ícone da classe do personagem que está equipando */
	UPROPERTY(meta = (BindWidget))
	UImage* CharacterClassIcon = nullptr;

private:
	/** Item atual sendo exibido */
	FInventoryItem CurrentItem;

	/** Slot de destino para equipar o item */
	EEquipmentSlot TargetSlot = EEquipmentSlot::Weapon;

	/** Personagem selecionado que está equipando */
	ARPGCharacter* SelectedCharacter = nullptr;

	/** Atualizar exibição com dados atuais */
	void UpdateDisplay();

	/** Handler para botão de equipar */
	UFUNCTION()
	void HandleEquipButtonClicked();

	/** Handler para hover no botão */
	UFUNCTION()
	void HandleButtonHovered();

	/** Handler para unhover no botão */
	UFUNCTION()
	void HandleButtonUnhovered();

	/** Obter nome amigável do slot */
	FString GetSlotDisplayName() const;
};
