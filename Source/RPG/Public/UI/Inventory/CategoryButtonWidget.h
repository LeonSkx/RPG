#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Core/InventoryEnums.h"
#include "CategoryButtonWidget.generated.h"

class UTextBlock;
class UImage;
class UButton;
class USizeBox;

/**
 * Widget de botão de categoria para o sistema de inventário
 * Estrutura: TXT + ICON + BUTTON
 * Reutilizável para qualquer categoria de inventário
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API UCategoryButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Configura o widget com dados da categoria */
	UFUNCTION(BlueprintCallable, Category = "Category Button")
	void SetupCategory(EItemCategory InCategory, const FString& InDisplayName);

	/** Obtém a categoria deste botão */
	UFUNCTION(BlueprintPure, Category = "Category Button")
	EItemCategory GetCategory() const { return Category; }

	/** Obtém o nome de exibição da categoria */
	UFUNCTION(BlueprintPure, Category = "Category Button")
	FString GetDisplayName() const { return DisplayName; }

	/** Define se o botão está selecionado */
	UFUNCTION(BlueprintCallable, Category = "Category Button")
	void SetSelected(bool bInSelected);
	
	/** Verifica se o botão está selecionado */
	UFUNCTION(BlueprintPure, Category = "Category Button")
	bool IsSelected() const { return bIsSelected; }
	
	/** Define o ícone da categoria */
	UFUNCTION(BlueprintCallable, Category = "Category Button")
	void SetCategoryIcon(UTexture2D* InIcon);

	/** Evento disparado quando o botão é clicado */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCategoryClicked, EItemCategory, Category);
	UPROPERTY(BlueprintAssignable, Category = "Category Button")
	FOnCategoryClicked OnCategoryClicked;

protected:
	/** Callback para clique no botão */
	UFUNCTION()
	void HandleButtonClicked();

private:
	/** Categoria representada por este botão */
	UPROPERTY(BlueprintReadOnly, Category = "Category Button", meta = (AllowPrivateAccess = "true"))
	EItemCategory Category = EItemCategory::None;

	/** Nome de exibição da categoria */
	UPROPERTY(BlueprintReadOnly, Category = "Category Button", meta = (AllowPrivateAccess = "true"))
	FString DisplayName;

	/** Estado de seleção do botão */
	UPROPERTY(BlueprintReadOnly, Category = "Category Button", meta = (AllowPrivateAccess = "true"))
	bool bIsSelected = false;

	// === COMPONENTES UI ===
	
	/** Container principal para controle de tamanho */
	UPROPERTY(meta = (BindWidget))
	USizeBox* MainContainer = nullptr;
	
	/** Texto da categoria */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CategoryText = nullptr;

	/** Ícone da categoria */
	UPROPERTY(meta = (BindWidget))
	UImage* CategoryIcon = nullptr;

	/** Botão clicável */
	UPROPERTY(meta = (BindWidget))
	UButton* CategoryButton = nullptr;
};
