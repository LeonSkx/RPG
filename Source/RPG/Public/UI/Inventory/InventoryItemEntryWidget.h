#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/RPGUserWidget.h"
#include "Inventory/Core/InventoryTypes.h"
#include "InventoryItemEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UTexture2D;
class USizeBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemSelected, const FInventoryItem&, Item);

UCLASS(BlueprintType, Blueprintable)
class RPG_API UInventoryItemEntryWidget : public URPGUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Entry")
	void Setup(const FInventoryItem& InItem);

	/** Disparado quando o botão do entry é clicado */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Entry")
	FOnInventoryItemSelected OnItemSelected;

	/** Obtém o item atual deste entry */
	UFUNCTION(BlueprintPure, Category = "Inventory|Entry")
	const FInventoryItem& GetCurrentItem() const { return CurrentItem; }

	/** Define visual de seleção deste entry (mostra/oculta o overlay). */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Entry|Selection")
	void SetSelectedVisual(bool bSelected);

protected:
	UFUNCTION()
	void HandleClicked();

	UFUNCTION()
	void HandleHovered();

	/** Referência do item representado neste entry */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry")
	FInventoryItem CurrentItem;

	/** Botão para selecionar o item */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry", meta = (BindWidget))
	UButton* SelectButton = nullptr;

	/** Texto com o nome do item */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry", meta = (BindWidget))
	UTextBlock* ItemNameText = nullptr;

	/** Texto com a quantidade (ex.: X01). Opcional no layout */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* QuantityText = nullptr;

	/** Ícone do tipo ou do próprio item */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Entry", meta = (BindWidget))
	UImage* TypeIcon = nullptr;

	/** Container opcional para forçar tamanho do ícone */
	UPROPERTY(meta = (BindWidgetOptional))
	USizeBox* IconSizeBox = nullptr;

	/** Fallback: mapa de ícones por categoria caso o ItemData não tenha TypeIcon */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Entry|Icons")
	TMap<EItemCategory, TSoftObjectPtr<UTexture2D>> CategoryIconMap;

	/** Cache de texturas carregadas para evitar recarregamento desnecessário */
	UPROPERTY(Transient)
	TMap<FString, TWeakObjectPtr<UTexture2D>> TextureCache;

	/** Imagem opcional que será exibida quando este entry estiver selecionado (bind no BP). */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "Inventory|Entry|Selection")
	UImage* SelectedOverlayImage = nullptr;

private:
	/** Obtém textura do cache ou carrega e cacheia */
	UTexture2D* GetOrLoadTexture(const TSoftObjectPtr<UTexture2D>& TexturePtr);

    // Removido: ícone selecionado padrão será definido no BP via brush do SelectedOverlayImage
};


