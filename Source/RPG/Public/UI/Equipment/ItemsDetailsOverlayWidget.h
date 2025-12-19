#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/RPGUserWidget.h"
#include "Inventory/Core/InventoryTypes.h"
#include "ItemsDetailsOverlayWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;

// Disparado quando o widget exibe um equipamento válido (selecionado)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentDetailsSelected);

/**
 * Widget que exibe os detalhes de um equipamento selecionado
 * Operação: recebe um item via Setup() e exibe suas informações como equipamento
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API UItemsDetailsOverlayWidget : public URPGUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Evento emitido quando os detalhes são mostrados para um equipamento válido */
	UPROPERTY(BlueprintAssignable, Category = "Equipment Details")
	FOnEquipmentDetailsSelected OnSelected;

	/** Configura o widget com os dados do equipamento selecionado */
	UFUNCTION(BlueprintCallable, Category = "Equipment Details")
	void Setup(const FInventoryItem& InItem);

	/** Limpa os dados exibidos */
	UFUNCTION(BlueprintCallable, Category = "Equipment Details")
	void Clear();

	/** Verifica se o widget está exibindo algum equipamento */
	UFUNCTION(BlueprintPure, Category = "Equipment Details")
	bool HasItem() const { return CurrentItem.IsValid(); }

	/** Obtém o equipamento atualmente exibido (somente leitura) */
	UFUNCTION(BlueprintPure, Category = "Equipment Details")
	const FInventoryItem& GetCurrentItem() const { return CurrentItem; }

protected:
	/** Atualiza a interface com os dados do equipamento atual */
	void UpdateDisplay();

	/** Retorna a string de exibição do tipo do equipamento usando sistema dinâmico */
	FString GetEquipmentTypeDisplayString(const UItemDataAsset* ItemData) const;

	/** Referência do equipamento atualmente exibido */
	UPROPERTY(BlueprintReadOnly, Category = "Equipment Details")
	FInventoryItem CurrentItem;

	/** Texto com o nome do equipamento */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText = nullptr;

	/** Texto com o tipo/categoria do equipamento */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemTypeText = nullptr;

	/** Texto com a descrição do equipamento */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemDescriptionText = nullptr;

	/** Imagem com o ícone do tipo do equipamento */
	UPROPERTY(meta = (BindWidget))
	UImage* ItemTypeIcon = nullptr;

	/** Botão para fechar o overlay (opcional) */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseButton = nullptr;

private:
	UFUNCTION()
	void HandleCloseButtonClicked();
};
