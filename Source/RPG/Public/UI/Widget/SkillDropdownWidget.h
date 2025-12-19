// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "UI/Widget/SkillDisplayData.h"
#include "SkillDropdownWidget.generated.h"

class USkillWidget;
class USkillItemWidget;
class USkillTreeWidgetController;
class ARPGCharacter;

/**
 * Widget dropdown para lista de habilidades
 * Mostra habilidades disponíveis para equipar
 */
UCLASS()
class RPG_API USkillDropdownWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // === CONFIGURAÇÃO ===
    
    /** Define o SkillWidget pai */
    UFUNCTION(BlueprintCallable, Category = "Skill Dropdown")
    void SetParentSkillWidget(USkillWidget* InSkillWidget);
    
    /** Mostra a lista de habilidades */
    UFUNCTION(BlueprintCallable, Category = "Skill Dropdown")
    void ShowSkillList();
    
    /** Esconde a lista de habilidades */
    UFUNCTION(BlueprintCallable, Category = "Skill Dropdown")
    void HideSkillList();
    
    /** Equipa uma habilidade */
    UFUNCTION(BlueprintCallable, Category = "Skill Dropdown")
    void EquipSkill(const FName& SkillID);
    
    /** Define o slot atual para equipamento */
    UFUNCTION(BlueprintCallable, Category = "Skill Dropdown")
    void SetCurrentSlotID(const FName& SlotID);
    
    /** Define o personagem alvo pelo CharacterID (com validação) */
    UFUNCTION(BlueprintCallable, Category = "Skill Dropdown")
    void SetTargetCharacterID(const FName& CharacterID);
    
    /** Busca um personagem pelo CharacterID (com validação) */
    UFUNCTION(BlueprintCallable, Category = "Skill Dropdown")
    ARPGCharacter* GetCharacterByID(const FName& CharacterID) const;

    // === EVENTOS ===
    
    /** Delegate para mudança de visibilidade do dropdown */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropdownVisibilityChanged, bool, bIsVisible);
    
    /** Evento disparado quando a visibilidade do dropdown muda */
    UPROPERTY(BlueprintAssignable, Category = "Skill Dropdown")
    FOnDropdownVisibilityChanged OnDropdownVisibilityChanged;
    
    /** Delegate para mudança de personagem no dropdown */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDropdownCharacterChanged, FName, CharacterID);
    
    /** Evento disparado quando o personagem muda no dropdown */
    UPROPERTY(BlueprintAssignable, Category = "Skill Dropdown")
    FOnDropdownCharacterChanged OnDropdownCharacterChanged;

    // === INTERNAL ===
    
    /** Chamado quando um item é selecionado */
    UFUNCTION()
    void OnSkillItemSelected(USkillItemWidget* SelectedItem);

protected:
    // === WIDGETS ===
    
    /** ScrollBox para a lista de habilidades */
    UPROPERTY(meta = (BindWidget))
    UScrollBox* SkillListScrollBox;
    
    /** Classe do widget de item de habilidade */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Dropdown")
    TSubclassOf<USkillItemWidget> SkillItemWidgetClass;
    
    /** Estilo para texto de lista vazia */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Dropdown")
    FTextBlockStyle EmptyTextStyle;

    // === DADOS ===
    
    /** Se o dropdown está visível */
    UPROPERTY()
    bool bIsVisible = false;
    
    /** Flag para controlar refresh concorrente */
    UPROPERTY()
    bool bIsRefreshing = false;
    
    /** Referência ao SkillWidget pai */
    UPROPERTY()
    TObjectPtr<USkillWidget> ParentSkillWidget;
    
    /** Controller para dados de habilidades */
    UPROPERTY()
    TObjectPtr<USkillTreeWidgetController> Controller;
    
    /** Item atualmente selecionado */
    UPROPERTY()
    TObjectPtr<USkillItemWidget> CurrentSelectedItem;
    
    /** Slot atual para equipamento */
    UPROPERTY()
    FName CurrentSlotID;
    
    /** CharacterID atual do personagem alvo */
    UPROPERTY()
    FName CurrentCharacterID;

    // === TIMER ===
    
    /** Timer para atualizações periódicas */
    FTimerHandle RefreshTimerHandle;

protected:
    // === HELPERS ===
    
    /** Inicializa o controller */
    void InitializeController();
    
    /** Obtém o personagem ativo */
    ARPGCharacter* GetActiveCharacter();
    
    /** Obtém habilidades desbloqueadas */
    TArray<FName> GetUnlockedSkills();
    
    /** Cria widgets de itens de habilidade */
    void CreateSkillItemWidgets(const TArray<FName>& SkillIDs);
    
    /** Obtém dados de display de uma habilidade */
    FSkillDisplayData GetSkillDisplayData(const FName& SkillID);
    
    /** Encontra SlotID para uma habilidade */
    FName FindSlotIDForSkill(const FName& SkillID);
    
    /** Atualiza a lista de habilidades (com controle de concorrência) */
    void RefreshSkillList();
    
    /** Limpa a lista de habilidades */
    void ClearSkillList();
    
    /** Cria mensagem de lista vazia */
    void CreateEmptyListMessage();
    
    /** Atualização periódica */
    void PeriodicRefresh();
    
    /** Conecta aos eventos do SkillTree */
    void ConnectToSkillTreeEvents();
    
    /** Desconecta dos eventos do SkillTree */
    void DisconnectFromSkillTreeEvents();

    // === EVENTOS ===
    
    /** Handler para habilidade desbloqueada */
    UFUNCTION()
    void OnSkillUnlocked(ARPGCharacter* Character, FName SkillID);
    
    // REMOVIDO: Eventos de equipamento movidos para SkillEquipmentComponent
}; 