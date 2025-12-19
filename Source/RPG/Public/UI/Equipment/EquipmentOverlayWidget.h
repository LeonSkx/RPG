#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/RPGUserWidget.h"
#include "Inventory/Core/InventoryEnums.h"
#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquipmentComparisonComponent.h"
#include "EquipmentOverlayWidget.generated.h"

class UTextBlock;
class UButton;
class UScrollBox;
class USizeBox;
class UImage;
class UInventorySubsystem;
class UItemListEntryWidget;
class UEquipmentComponent;
class UItemsDetailsOverlayWidget;
class UCharacterSelectorWidget;
class UHorizontalBox;
class UPartySubsystem;
class UItemConfirmationOverlay;
class UEquipmentStatusWidget;

/**
 * Widget principal do sistema de equipamentos
 * Gerencia a interface de equipamentos do jogador
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API UEquipmentOverlayWidget : public URPGUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	/** Título do overlay */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* TitleText = nullptr;

	/** Texto do slot da arma */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponSlotText = nullptr;

	/** Botão do slot da arma - exibe armas disponíveis */
	UPROPERTY(meta = (BindWidget))
	UButton* WeaponSlotButton = nullptr;

	/** Texto do slot da armadura */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ArmorSlotText = nullptr;

	/** Botão do slot da armadura - exibe armaduras disponíveis */
	UPROPERTY(meta = (BindWidget))
	UButton* ArmorSlotButton = nullptr;

	/** Texto do slot do acessório */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AccessorySlotText = nullptr;

	/** Botão do slot do acessório - exibe acessórios disponíveis */
	UPROPERTY(meta = (BindWidget))
	UButton* AccessorySlotButton = nullptr;

	/** Texto do slot da bota */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BootsSlotText = nullptr;

	/** Botão do slot da bota - exibe botas disponíveis */
	UPROPERTY(meta = (BindWidget))
	UButton* BootsSlotButton = nullptr;

	/** Texto do slot do anel */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RingSlotText = nullptr;

	/** Botão do slot do anel - exibe anéis disponíveis */
	UPROPERTY(meta = (BindWidget))
	UButton* RingSlotButton = nullptr;

	/** Texto de informação do personagem selecionado */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CharacterName = nullptr;

	/** Ícones dos tipos para cada slot */
	UPROPERTY(meta = (BindWidget))
	UImage* WeaponSlotIcon = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* ArmorSlotIcon = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* AccessorySlotIcon = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* BootsSlotIcon = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* RingSlotIcon = nullptr;

	/** Imagens de hover para cada slot */
	UPROPERTY(meta = (BindWidget))
	UImage* WeaponSlotHoverImage = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* ArmorSlotHoverImage = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* AccessorySlotHoverImage = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* BootsSlotHoverImage = nullptr;

	UPROPERTY(meta = (BindWidget))
	UImage* RingSlotHoverImage = nullptr;

	/** ScrollBox para listar itens disponíveis (compartilhado) */
	/** Container SizeBox para a lista de itens */
	UPROPERTY(meta = (BindWidget))
	USizeBox* ItemsListContainer = nullptr;

	/** Texto informativo da lista de itens */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ListTxt = nullptr;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ItemsScrollBox = nullptr;

	/** Container para o widget de detalhes da arma */
	UPROPERTY(meta = (BindWidget))
	USizeBox* DetailsContainer = nullptr;

	/** Container para o overlay de confirmação de item compartilhado */
	UPROPERTY(meta = (BindWidget))
	USizeBox* ConfirmationContainer = nullptr;

	/** Container para o widget de status de equipamento (dano total) */
	UPROPERTY(meta = (BindWidget))
	USizeBox* EquipmentStatusContainer = nullptr;

	/** Botão para fechar o overlay (opcional) */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* CloseButton = nullptr;

	/** Classe do widget de entrada de item (genérico) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment List")
	TSubclassOf<UItemListEntryWidget> ItemEntryClass;

	/** Classe do widget de detalhes de equipamento */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Details")
	TSubclassOf<UItemsDetailsOverlayWidget> EquipmentDetailsClass;

	/** Classe do widget de seleção de personagem */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Selection")
	TSubclassOf<UCharacterSelectorWidget> CharacterSelectorClass;



	/** Classe do widget de confirmação de item compartilhado */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Confirmation")
	TSubclassOf<UItemConfirmationOverlay> ItemConfirmationClass;

	/** Classe do widget de status de equipamento */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment Status")
	TSubclassOf<UEquipmentStatusWidget> EquipmentStatusClass;

	/** Container para os seletores de personagem */
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* CharacterSelectorsContainer = nullptr;

	/** Imagem para exibir o personagem 3D selecionado */


	/** Array de widgets de seleção de personagem */
	TArray<TObjectPtr<UCharacterSelectorWidget>> CharacterSelectorWidgets;



	/** Personagem atualmente selecionado para gerenciar equipamentos */
	UPROPERTY(BlueprintReadOnly, Category = "Character Selection")
	ARPGCharacter* SelectedCharacter = nullptr;

private:
	/** Inicializar componentes básicos */
	void InitializeComponents();

	/** Array de widgets de item criados */
	TArray<TObjectPtr<UItemListEntryWidget>> ItemEntryWidgets;

	/** Widget de detalhes do equipamento (criado dinamicamente) */
	TObjectPtr<UItemsDetailsOverlayWidget> EquipmentDetailsWidget;

	/** Widget de confirmação de item compartilhado (criado dinamicamente) */
	TObjectPtr<UItemConfirmationOverlay> ItemConfirmationWidget;

	/** Widget de status de equipamento (criado dinamicamente) */
	TObjectPtr<UEquipmentStatusWidget> EquipmentStatusWidget;

	/** Componente de comparação do personagem selecionado */
	UPROPERTY(BlueprintReadOnly, Category="Equipment Overlay", meta=(AllowPrivateAccess="true"))
	UEquipmentComparisonComponent* ComparisonComponent = nullptr;

	/** Handlers para clique nos botões dos slots */
	UFUNCTION()
	void HandleWeaponSlotButtonClicked();
	
	UFUNCTION()
	void HandleArmorSlotButtonClicked();
	
	UFUNCTION()
	void HandleAccessorySlotButtonClicked();
	
	UFUNCTION()
	void HandleBootsSlotButtonClicked();
	
	UFUNCTION()
	void HandleRingSlotButtonClicked();

	/** Handlers para hover nos botões dos slots - mostrar item equipado */
	UFUNCTION()
	void HandleWeaponSlotButtonHovered();
	
	UFUNCTION()
	void HandleArmorSlotButtonHovered();
	
	UFUNCTION()
	void HandleAccessorySlotButtonHovered();
	
	UFUNCTION()
	void HandleBootsSlotButtonHovered();
	
	UFUNCTION()
	void HandleRingSlotButtonHovered();

	/** Handlers para quando sai do hover nos botões dos slots */
	UFUNCTION()
	void HandleWeaponSlotButtonUnhovered();
	
	UFUNCTION()
	void HandleArmorSlotButtonUnhovered();
	
	UFUNCTION()
	void HandleAccessorySlotButtonUnhovered();
	
	UFUNCTION()
	void HandleBootsSlotButtonUnhovered();
	
	UFUNCTION()
	void HandleRingSlotButtonUnhovered();

	/** Popular ScrollBox com itens de uma categoria específica */
	void PopulateItemsList(EInventoryFilterCategory FilterCategory, EEquipmentSlot TargetSlot);

	/** Limpar lista de itens */
	void ClearItemsList();

	/** Configurar escuta de eventos do EquipmentComponent */
	void SetupEquipmentEvents();

	/** Atualizar displays de todos os slots */
	void UpdateAllSlotsDisplay();
	
	/** Atualizar display de um slot específico */
	void UpdateSlotDisplay(EEquipmentSlot EquipSlot);

	/** Atualizar todos os widgets que dependem de equipamentos */
	void UpdateAllEquipmentDependentWidgets();

	/** Atualizar ícones de classe de todos os itens na lista */
	void UpdateItemListIcons();

	/** Configurar e exibir o personagem 3D no viewport */


	/** Handler para quando item é equipado */
	UFUNCTION()
	void OnItemEquipped(EEquipmentSlot EquipSlot, UEquippedItem* EquippedItem);

	/** Handler para quando item é desequipado */
	UFUNCTION()
	void OnItemUnequipped(EEquipmentSlot EquipSlot, const FInventoryItem& UnequippedItem);

	/** Handler para hover em item da lista */
	UFUNCTION()
	void OnItemHovered(const FInventoryItem& HoveredItem);

	/** Handler para quando item da lista é equipado (fechar lista) */
	UFUNCTION()
	void OnItemEquippedFromList(const FInventoryItem& EquippedItem);

	/** Limpar hover de item da lista (quando hover em slot fixo) */
	void ClearItemListHover();

	/** Aplicar seleção de item (exibir detalhes) */
	void ApplyItemSelection(const FInventoryItem& SelectedItem);

	/** Mostrar detalhes do item equipado no slot especificado */
	void ShowEquippedItemDetails(EEquipmentSlot EquipSlot);

	/** Obter nome amigável do slot para logs */
	FString GetSlotDisplayName(EEquipmentSlot EquipSlot) const;

	/** Configurar ícones dos slots baseado nos itens equipados */
	void SetupSlotIcons();

	/** Configurar ícone de um slot específico */
	void SetupSlotIcon(EEquipmentSlot EquipSlot);

	/** Configurar ícone padrão para slot vazio */
	void SetDefaultSlotIcon(UImage* SlotIcon, EEquipmentSlot EquipSlot);

	/** Controlar visual de hover de um slot específico */
	void SetSlotHoverVisual(EEquipmentSlot EquipSlot, bool bHovered);

	/** Limpar todos os visuais de hover */
	void ClearAllSlotHoverVisuals();

	/** Handler para botão de fechar */
	UFUNCTION()
	void HandleCloseButtonClicked();

	/** Fechar a lista de itens (ocultar ScrollBox e Details) */
	void CloseItemsList(bool bPreserveSlotHover = false);

	/** Verificar se a lista está visível */
	bool IsListVisible() const;

	/** Auto-selecionar o slot Weapon ao abrir o overlay */
	void AutoSelectWeaponSlot();
	
	/** Ativar hover automático em um slot (usado após equipar item) */
	void ActivateSlotHover(EEquipmentSlot EquipSlot);

	/** Configurar sistema de seleção de personagens */
	void SetupCharacterSelection();

	/** Popular container com seletores de personagem */
	void PopulateCharacterSelectors();

	/** Limpar seletores de personagem */
	void ClearCharacterSelectors();

	/** Atualizar estado visual de todos os seletores */
	void UpdateAllSelectorsVisualState();

	/** Limpar e destruir o widget de detalhes do equipamento */
	void ClearEquipmentDetailsWidget();

	/** Criar e configurar o widget de status de equipamento */
	void SetupEquipmentStatusWidget();

	/** Limpar o widget de status de equipamento */
	void ClearEquipmentStatusWidget();

	/** Configurar estado inicial oculto dos elementos da lista */
	void SetupInitialListState();

	/** Função centralizada para lidar com hover dos slots */
	void HandleSlotHovered(EEquipmentSlot SlotType);







	/** Handler para quando personagem é selecionado */
	UFUNCTION()
	void OnCharacterSelected(ARPGCharacter* SelectedChar);

	/** Atualizar personagem selecionado */
	void UpdateSelectedCharacter(ARPGCharacter* NewSelectedCharacter);

	/** Obter nome de exibição do personagem */
	FString GetCharacterDisplayName(ARPGCharacter* Character) const;

	/** Obter EquipmentComponent do personagem atualmente selecionado */
	UEquipmentComponent* GetSelectedCharacterEquipmentComponent() const;

	/** Handler para solicitação de equipamento de item (NOVO) */
	UFUNCTION()
	void OnEquipItemRequested(const FInventoryItem& ItemToEquip, EEquipmentSlot TargetSlot);

	/** Verificar se item está sendo usado por outro personagem */
	bool CheckItemConflict(const FInventoryItem& ItemToEquip, EEquipmentSlot TargetSlot, ARPGCharacter*& OutConflictCharacter);

	/** Mostrar overlay de confirmação para item compartilhado */
	void ShowItemConfirmationOverlay(ARPGCharacter* ConflictCharacter, const FInventoryItem& ItemToEquip, EEquipmentSlot TargetSlot);

	/** Handler para quando confirmação é resolvida */
	UFUNCTION()
	void OnItemConfirmationResolved(bool bConfirmed, ARPGCharacter* NewUser, ARPGCharacter* PreviousUser);

	/** Simula equipamento para comparação de stats */
	UFUNCTION()
	void SimulateEquipmentForComparison(const FInventoryItem& ItemToSimulate);

	/** Limpa simulação de equipamento */
	UFUNCTION()
	void ClearEquipmentSimulation();

	/** Ordena membros da party na ordem desejada: Yumi → Axix → Yoshino → Shimako */
	TArray<ARPGCharacter*> OrderPartyMembers(const TArray<ARPGCharacter*>& PartyMembers);

private:
	/** Slot com imagem de hover ativa (acompanha os detalhes exibidos) */
	EEquipmentSlot ActiveHoverSlot = EEquipmentSlot::Weapon;
	
	/** Auto-seleção ativa no slot Weapon ao abrir o overlay */
	bool bIsAutoSelectionActive = false;

	/** Primeiro item tem auto-hover inicial ativo */
	bool bFirstItemAutoHover = false;

	/** Item da lista com hover ativo (similar ao ActiveHoverSlot) */
	UItemListEntryWidget* ActiveHoverItem = nullptr;

	/** Slot que tem a lista atualmente aberta */
	EEquipmentSlot CurrentOpenSlot = EEquipmentSlot::Weapon;
};
