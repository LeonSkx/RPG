// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Engine/Texture2D.h"
#include "SkillItemWidget.generated.h"

class USkillDropdownWidget;

/**
 * Widget para um item de habilidade no dropdown
 * Permite selecionar e equipar habilidades
 */
UCLASS()
class RPG_API USkillItemWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // === CONFIGURAÇÃO ===
    
    /** Define os dados da habilidade */
    UFUNCTION(BlueprintCallable, Category = "Skill Item")
    void SetSkillData(const FName& InSkillID, const FText& InSkillName, UTexture2D* InSkillIcon, bool bInIsEquipped);
    
    /** Define o dropdown pai */
    UFUNCTION(BlueprintCallable, Category = "Skill Item")
    void SetParentDropdown(USkillDropdownWidget* InParentDropdown);
    
    /** Define se o item está selecionado */
    UFUNCTION(BlueprintCallable, Category = "Skill Item")
    void SetSelected(bool bInSelected);

    // === GETTERS ===
    
    /** Verifica se está selecionado */
    UFUNCTION(BlueprintPure, Category = "Skill Item")
    bool IsSelected() const;
    
    /** Verifica se está equipado */
    UFUNCTION(BlueprintPure, Category = "Skill Item")
    bool IsEquipped() const;
    
    /** Obtém o ID da habilidade */
    UFUNCTION(BlueprintPure, Category = "Skill Item")
    FName GetSkillID() const;
    
    /** Obtém o nome da habilidade */
    UFUNCTION(BlueprintPure, Category = "Skill Item")
    FText GetSkillName() const;

protected:
    // === WIDGETS ===
    
    /** Ícone da habilidade */
    UPROPERTY(meta = (BindWidget))
    UImage* SkillIcon;
    
    /** Nome da habilidade */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SkillNameText;
    
    /** Botão para equipar */
    UPROPERTY(meta = (BindWidget))
    UButton* EquipButton;

    // === DADOS ===
    
    /** ID da habilidade */
    UPROPERTY()
    FName SkillID;
    
    /** Nome da habilidade */
    UPROPERTY()
    FText SkillName;
    
    /** Textura do ícone */
    UPROPERTY()
    TObjectPtr<UTexture2D> SkillIconTexture;
    
    /** Se está selecionado */
    UPROPERTY()
    bool bIsSelected = false;
    
    /** Se está equipado */
    UPROPERTY()
    bool bIsEquipped = false;
    
    /** Tempo do último clique */
    UPROPERTY()
    double LastClickTime = 0.0;
    
    /** Referência ao dropdown pai */
    UPROPERTY()
    TObjectPtr<USkillDropdownWidget> ParentDropdown;

    // === TIMER ===
    
    /** Timer para reset de seleção */
    FTimerHandle SelectionResetTimer;

protected:
    // === EVENTOS ===
    
    /** Handler para clique no botão */
    UFUNCTION()
    void OnButtonClicked();
    
    /** Handler para clique no botão de equipar */
    UFUNCTION()
    void OnEquipButtonClicked();

    // === VISUAL ===
    
    /** Atualiza o visual do widget */
    void UpdateVisual();
    
    /** Atualiza o ícone da habilidade */
    void UpdateSkillIcon();
    
    /** Atualiza o nome da habilidade */
    void UpdateSkillName();
    
    /** Atualiza o estado visual */
    void UpdateVisualState();
    
    /** Atualiza o estado do botão */
    void UpdateButtonState();
    
    /** Atualiza o background */
    void UpdateBackground();
    
    /** Verifica timeout de seleção */
    void CheckSelectionTimeout();
}; 