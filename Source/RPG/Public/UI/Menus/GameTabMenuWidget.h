#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameTabMenuWidget.generated.h"

class UVerticalBox;
class UWidgetSwitcher;
class UButton;
class UHorizontalBox;
class ARPGPlayerController;
class ARPGHUD;

/**
 * Sistema de menu por abas
 * Funcionalidades:
 * - Abertura/fechamento de menu
 * - Sistema de abas dinâmico
 * - Integração com Enhanced Input
 */
UCLASS()
class RPG_API UGameTabMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGameTabMenuWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// === CONTROLE DE MENU ===
	
	/** Fecha menu */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	void CloseMenu();

	/** Mostra/oculta o container de botões de abas, se existir */
	UFUNCTION(BlueprintCallable, Category = "Menu|Tabs")
	void ShowTabButtons(bool bShow);

protected:
	// === WIDGETS DO SISTEMA DE ABAS ===
	
	/** Switcher para trocar conteúdo das abas */
	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* TabContentSwitcher;

	/** Container opcional onde os botões de abas ficam (para controlar visibilidade de todos de uma vez) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> TabButtonsContainer;

	/** Botão de voltar para o estado inicial (ver apenas abas) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BackButton;

	// === BOTÕES DAS ABAS ===
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StatusButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkillsButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkillTreeButton; // Mantido apenas para compatibilidade; desabilitado em runtime
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> InventoryButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RecordsButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MapButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PartyButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SystemButton;

	// === CLASSES DE CONTEÚDO DAS ABAS ===
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab Content")
	TSubclassOf<UUserWidget> StatusContentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab Content")
	TSubclassOf<UUserWidget> SkillsContentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab Content")
	TSubclassOf<UUserWidget> SkillTreeContentClass; // Ignorado; árvore removida
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab Content")
	TSubclassOf<UUserWidget> InventoryContentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab Content")
	TSubclassOf<UUserWidget> RecordsContentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab Content")
	TSubclassOf<UUserWidget> MapContentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab Content")
	TSubclassOf<UUserWidget> PartyContentClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab Content")
	TSubclassOf<UUserWidget> SystemContentClass;

	/** Habilita carregamento sob demanda */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Lazy Loading")
	bool bEnableLazyLoading = true;
	
	/** Número máximo de widgets em cache */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Lazy Loading")
	int32 MaxCachedWidgets = 1;

	// === CALLBACKS DOS BOTÕES ===
	
	UFUNCTION()
	void OnStatusButtonClicked();
	
	UFUNCTION()
	void OnSkillsButtonClicked();
	
	UFUNCTION()
	void OnSkillTreeButtonClicked();
	
	UFUNCTION()
	void OnInventoryButtonClicked();
	
	UFUNCTION()
	void OnRecordsButtonClicked();
	
	UFUNCTION()
	void OnMapButtonClicked();
	
	UFUNCTION()
	void OnPartyButtonClicked();
	
	UFUNCTION()
	void OnSystemButtonClicked();

	// === SISTEMA INTERNO ===
	
	/** Inicializa todas as abas */
	void InitializeTabs();

	/** Seleciona uma aba padrão seguindo ordem de preferência */
	void SelectDefaultTab();

	/** Retorna ao estado inicial (somente botões de abas visíveis) */
	void ReturnToHeaderState();

	UFUNCTION()
	void OnBackButtonClicked();
	
	/** Define qual aba está ativa */
	void SetActiveTab(UButton* TargetButton);
	
	/** Atualiza estilos visuais dos botões */
	void UpdateTabButtonStyles();
	
	/** Carrega widget sob demanda */
	void LoadTabOnDemand(UButton* TargetButton);
	
	/** Gerencia cache de widgets */
	void ManageWidgetCache();

private:
	// === MAPEAMENTOS INTERNOS ===
	
	/** Mapeia botões para classes de conteúdo */
	TMap<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>> ButtonToClassMap;
	
	/** Mapeia botões para instâncias de widgets */
	TMap<TWeakObjectPtr<UButton>, TWeakObjectPtr<UUserWidget>> ButtonToInstanceMap;
	
	/** Botão atualmente ativo */
	TWeakObjectPtr<UButton> ActiveTabButton;
	
	/** Mapeia botões para timestamps de último acesso */
	TMap<TWeakObjectPtr<UButton>, double> LastAccessTimeMap;
}; 