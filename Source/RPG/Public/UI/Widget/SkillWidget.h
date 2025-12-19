// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/SizeBox.h"
#include "GameplayTagContainer.h"
#include "SkillWidget.generated.h"

class USkillTreeWidgetController;
class USlotSkillWidget;

// Delegates para eventos do SkillWidget
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillWidgetSlotClicked, FName, SlotID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillWidgetSkillEquipped, FName, SlotID, FName, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillWidgetSkillUnequipped, FName, SlotID, FName, SkillID);

// Delegate para mudança de personagem
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillWidgetCharacterChanged, FName, CharacterID);

/**
 * Widget principal para gerenciar equipamento de habilidades
 * Suporta múltiplos slots e múltiplos personagens
 */
UCLASS()
class RPG_API USkillWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    USkillWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // === CONFIGURAÇÃO ===
    
    /** Define o controller para este widget */
    UFUNCTION(BlueprintCallable, Category = "Widget Controller")
    void SetController(USkillTreeWidgetController* InController);
    
    /** Inicializa o widget com um controller */
    UFUNCTION(BlueprintCallable, Category = "Widget Controller")
    void InitializeWithController(USkillTreeWidgetController* InController);
    
    /** Inicializa o controller automaticamente */
    UFUNCTION(BlueprintCallable, Category = "Widget Controller")
    void InitializeController();
    
    /** Obtém o controller atual */
    UFUNCTION(BlueprintPure, Category = "Skill Widget")
    USkillTreeWidgetController* GetController() const { return SkillTreeController; }

    // === GERENCIAMENTO DE SLOTS ===
    
    /** Adiciona um slot ao widget */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void AddSlot(USlotSkillWidget* SlotWidget);
    
    /** Remove um slot do widget */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void RemoveSlot(USlotSkillWidget* SlotWidget);
    
    /** Conecta todos os slots automaticamente */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void ConnectAllSlots();
    
    /** Desconecta todos os slots */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void DisconnectAllSlots();
    
    /** Conecta um array de slots */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void ConnectSlotsArray(const TArray<USlotSkillWidget*>& SlotsArray);

    // === EVENTOS DE SLOT ===
    
    /** Chamado quando um slot é clicado */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void OnSlotClicked(const FName& SlotID);
    
    /** Chamado quando uma habilidade é equipada */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void OnSkillEquipped(const FName& SlotID, const FName& SkillID);
    
    /** Chamado quando uma habilidade é desequipada */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void OnSkillUnequipped(const FName& SlotID, const FName& SkillID);

    // === WIDGET DROPDOWN ===
    
    /** Abre um widget dropdown para um slot específico */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void OpenSkillDropdown(const FName& SlotID);
    
    /** Fecha o dropdown atual */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void CloseSkillDropdown();
    
    /** Equipa uma habilidade */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void EquipSkill(const FName& SkillID);
    
    /** Equipa uma habilidade em um slot específico */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void EquipSkillInSlot(const FName& SkillID, const FName& SlotID);
    
    /** Unequipa uma habilidade de um slot específico */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void UnequipSkillFromSlot(const FName& SlotID);
    
    /** Atualiza os ícones de todos os slots */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    void UpdateAllSlotIcons();

    // === PERSONAGEM ===
    
    /** Busca um personagem pelo CharacterID */
    UFUNCTION(BlueprintCallable, Category = "Skill Widget")
    ARPGCharacter* GetCharacterByID(const FName& CharacterID) const;
    
    /** Retorna o personagem atual */
    UFUNCTION(BlueprintPure, Category = "Skill Widget")
    ARPGCharacter* GetCurrentCharacter() const { return CurrentCharacter; }
    
    /** Delegate que ENVIA CharacterID quando personagem muda */
    UPROPERTY(BlueprintAssignable, Category = "Skill Widget")
    FOnSkillWidgetCharacterChanged OnCharacterChanged;

    // === DELEGATES ===
    
    /** Disparado quando um slot é clicado */
    UPROPERTY(BlueprintAssignable, Category = "Skill Widget|Events")
    FOnSkillWidgetSlotClicked OnSkillWidgetSlotClicked;
    
    /** Disparado quando uma habilidade é equipada */
    UPROPERTY(BlueprintAssignable, Category = "Skill Widget|Events")
    FOnSkillWidgetSkillEquipped OnSkillWidgetSkillEquipped;
    
    /** Disparado quando uma habilidade é desequipada */
    UPROPERTY(BlueprintAssignable, Category = "Skill Widget|Events")
    FOnSkillWidgetSkillUnequipped OnSkillWidgetSkillUnequipped;

protected:
    // === WIDGETS DO HEADER ===
    
    /** ComboBox para selecionar o personagem */
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* CharacterSelector;
    
    // === FUNÇÕES ===
    
    /** Inicializa o ComboBox com os personagens do grupo */
    void InitializeCharacterSelector();
    
    /** Atualiza o header com informações do personagem */
    void UpdateHeader();
    
    // === HANDLERS DE EVENTOS ===
    
    /** Handler para quando o personagem selecionado muda */
    UFUNCTION()
    void OnCharacterSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    // === CONTROLLER ===
    
    /** Referência ao SkillTreeWidgetController */
    UPROPERTY(BlueprintReadOnly, Category = "Skill Widget")
    TObjectPtr<USkillTreeWidgetController> SkillTreeController;
    
    /** Se o widget foi inicializado */
    UPROPERTY()
    bool bIsInitialized = false;

    // === SLOTS ===
    
    /** Lista de slots gerenciados */
    UPROPERTY()
    TArray<TObjectPtr<USlotSkillWidget>> ManagedSlots;

    /** Cache de InputTags por SlotID para otimização */
    UPROPERTY()
    TMap<FName, FGameplayTag> CachedSlotInputTags;

protected:
    // === HELPERS ===
    
    /** Valida se o controller está disponível */
    bool IsControllerValid() const;
    
    /** Atualiza o widget com dados do controller */
    void UpdateWidgetFromController();
    
    /** Obtém a InputTag de um slot específico */
    FGameplayTag GetSlotInputTag(const FName& SlotID) const;
    
    /** Popula o cache de InputTags para otimização */
    void PopulateInputTagCache();
    
    /** Conecta um slot individual */
    void ConnectSlot(USlotSkillWidget* SlotWidget);
    
    /** Desconecta um slot específico */
    void DisconnectSlot(USlotSkillWidget* SlotWidget);

private:
    // === DADOS INTERNOS ===
    
    /** Referência ao personagem atual */
    UPROPERTY()
    ARPGCharacter* CurrentCharacter;
    
    /** Mapa de nomes no ComboBox para referências de personagens */
    UPROPERTY()
    TMap<FString, ARPGCharacter*> CharacterMap;
}; 