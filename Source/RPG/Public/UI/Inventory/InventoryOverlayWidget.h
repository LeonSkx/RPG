#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Core/InventoryTypes.h"
#include "Inventory/Core/InventoryEnums.h"
#include "InventoryOverlayWidget.generated.h"

// Forward declarations
class UButton;
class UTextBlock;
class UImage;
class USizeBox;
class UHorizontalBox;
class UHorizontalBox;
class UItemsDetailsOverlayWidget;
class UInventorySubsystem;
class UItemDataAsset;
class UCategoryButtonWidget;
class USubtypeButtonWidget;
class UCategoryIconsDataAsset;
class UInventoryItemEntryWidget;
class UScrollBox;

/**
 * Widget principal do sistema de inventário - sistema limpo para nova implementação
 * Sistema de filtros e subtipos será reimplementado com nova semântica
 */
UCLASS()
class RPG_API UInventoryOverlayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // === CONFIGURAÇÃO BLUEPRINT ===
    
    // Lista de inventário
    UPROPERTY(meta = (BindWidget))
    UScrollBox* ItemsScrollBox;
    
    /** Classe do widget de entry para itens */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    TSubclassOf<UInventoryItemEntryWidget> ItemEntryClass;

    // Container de detalhes
    UPROPERTY(meta = (BindWidget))
    USizeBox* DetailsContainer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    TSubclassOf<UItemsDetailsOverlayWidget> ItemDetailsClass;

    // Título
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TitleText;

    // Nome do subtype selecionado
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SubtypeNameText;

    // === SISTEMA DE CATEGORIAS ===
    
    /** Container horizontal para os botões de categoria */
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* CategoryButtonsContainer = nullptr;

    /** Classe do widget de botão de categoria */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    TSubclassOf<UCategoryButtonWidget> CategoryButtonClass;
    
    /** Container horizontal para os botões de subtype */
    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* SubtypeButtonsContainer = nullptr;

    /** Classe do widget de botão de subtype */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    TSubclassOf<USubtypeButtonWidget> SubtypeButtonClass;
    
    /** Data Asset com ícones das categorias */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    UCategoryIconsDataAsset* CategoryIconsData = nullptr;
    
    /** Data Asset com ícones dos subtypes */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    UCategoryIconsDataAsset* SubtypeIconsData = nullptr;

    // === MÉTODOS PÚBLICOS ===
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void SetTitle(const FText& InTitle);



protected:
    virtual void NativeConstruct() override;

    // === CALLBACKS DOS EVENTOS ===
    
    UFUNCTION()
    void OnItemSelected(const FInventoryItem& SelectedItem);

    /** Callback para quando uma categoria é clicada */
    UFUNCTION()
    void OnCategoryClicked(EItemCategory Category);
    
    /** Callback para quando um subtype é clicado */
    UFUNCTION()
    void OnSubtypeClicked(const FString& SubtypeName);
    
    /** Aplica filtro por categoria na lista de inventário */
    void ApplyCategoryFilter(EItemCategory Category);
    
    /** Aplica filtro por subtype na lista de inventário */
    void ApplySubtypeFilter(const FString& SubtypeName);
    
    /** Exibe itens filtrados por categoria */
    void DisplayItemsByCategory(EItemCategory Category);
    
    /** Exibe itens filtrados por subtype */
    void DisplayItemsBySubtype(const FString& SubtypeName);
    
    /** Converte EItemCategory para EInventoryFilterCategory */
    EInventoryFilterCategory ConvertItemCategoryToFilterCategory(EItemCategory ItemCategory);
    
    /** Atualiza a seleção visual dos botões de categoria */
    void UpdateCategoryButtonSelection(EItemCategory SelectedCategory);
    
    /** Atualiza a seleção visual dos botões de subtype */
    void UpdateSubtypeButtonSelection(const FString& SelectedSubtype);
    
    /** Reseta o estado de auto-hover */
    void ResetAutoHover();
    
    /** Popula a lista de itens com base na categoria */
    void PopulateItemsList(EItemCategory Category);
    
    /** Popula a lista de itens com base no subtype */
    void PopulateItemsListBySubtype(const FString& SubtypeName);
    
    /** Limpa a lista de itens */
    void ClearItemsList();
    
    /** Limpa o hover da lista de itens */
    void ClearItemListHover();
    
    /** Cria os botões de subtype para uma categoria */
    void CreateSubtypeButtons(EItemCategory Category);
    
    /** Limpa os botões de subtype */
    void ClearSubtypeButtons();
    
    /** Obtém os subtypes disponíveis para uma categoria */
    TArray<FString> GetAvailableSubtypes(EItemCategory Category);
    
    /** Obtém o ícone para um subtype específico de forma robusta */
    UTexture2D* GetSubtypeIcon(EItemCategory Category, const FString& SubtypeName);

private:
    // === COMPONENTES DINÂMICOS ===
    
    UPROPERTY(Transient)
    UItemsDetailsOverlayWidget* ItemDetailsWidget;
    
    /** Array de entries de itens ativos */
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<UInventoryItemEntryWidget>> ItemEntries;
    
    /** Item atualmente em hover na lista */
    UPROPERTY(Transient)
    UInventoryItemEntryWidget* ActiveHoverItem = nullptr;
    
    /** Array de botões de subtype ativos */
    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<USubtypeButtonWidget>> SubtypeButtons;

    // === ESTADO DO SISTEMA ===
    
    /** Categoria atualmente selecionada */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
    EItemCategory CurrentSelectedCategory = EItemCategory::None;
    
    /** Subtype atualmente selecionado */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
    FString CurrentSelectedSubtype = TEXT("");
    
    /** Flag para controlar auto-hover inicial na primeira categoria */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
    bool bFirstCategoryAutoHover = false;
    
    /** Flag para controlar auto-hover inicial no primeiro item da lista */
    UPROPERTY(BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
    bool bFirstItemAutoHover = false;

    // === MÉTODOS PRINCIPAIS ===
    
    void InitializeComponents();
    void CreateCategoryButtons();
    void ApplyItemSelection(const FInventoryItem& SelectedItem);

    // === MÉTODOS AUXILIARES ===
     
    UInventorySubsystem* GetInventorySubsystem() const;
    
    /** Obtém o nome de exibição de uma categoria */
    FString GetCategoryDisplayName(EItemCategory Category);

public:
    // === MÉTODOS BLUEPRINT EXPOSTOS (para compatibilidade) ===
    

};