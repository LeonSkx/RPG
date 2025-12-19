// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Widget/SkillDisplayData.h"
#include "SkillNodeWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class USkillTreeWidgetController;
struct FSkillDisplayData;

// Delegate para mudança de estado de habilidade
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnSkillStateChanged_Event, 
    FName, SkillID, 
    FName, SlotID, 
    ESkillState, NewState, 
    bool, bIsUnlocked, 
    bool, bIsEquipped, 
    bool, bCanUnlock);

/**
 * Widget para representar um nó individual da árvore de habilidades
 * Recebe Controller do Content e apenas exibe dados
 */
UCLASS()
class RPG_API USkillNodeWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    
    /** Atualiza o visual do nó baseado no estado */
    UFUNCTION(BlueprintCallable, Category = "SkillNode")
    void UpdateVisual();
    
    /** Atualiza o CharacterID e reinicializa o nó */
    UFUNCTION(BlueprintCallable, Category = "SkillNode")
    void UpdateCharacterID(const FName& NewCharacterID);
    
    /** Inicializa o nó automaticamente */
    void InitializeNodeAuto();
    
    /** Retorna o SkillID calculado deste nó */
    UFUNCTION(BlueprintPure, Category = "SkillNode")
    FName GetSkillID() const { return CalculatedSkillID; }
    
    /** Retorna o nome da habilidade */
    UFUNCTION(BlueprintPure, Category = "SkillNode")
    FText GetSkillName() const;
    
    /** Retorna o SlotID do nó */
    UFUNCTION(BlueprintPure, Category = "SkillNode")
    FName GetSlotID() const { return SlotID; }
    
    /** Retorna dados de exibição da habilidade */
    UFUNCTION(BlueprintPure, Category = "SkillNode")
    FSkillDisplayData GetSkillDisplayData() const;
    
    /** Desbloqueia a habilidade deste node (pode ser chamada no Blueprint) */
    UFUNCTION(BlueprintCallable, Category = "SkillNode")
    bool UnlockSkill();

    /** Equipa a habilidade deste node */
    UFUNCTION(BlueprintCallable, Category = "SkillNode")
    bool EquipSkill();

    /** Desequipa a habilidade deste node */
    UFUNCTION(BlueprintCallable, Category = "SkillNode")
    bool UnequipSkill();

    /** Desbloqueia automaticamente habilidades sem pré-requisitos */
    UFUNCTION(BlueprintCallable, Category = "SkillNode")
    void UnlockSkillsWithoutPrerequisites();
    
    /** Define o controller da árvore de habilidades */
    UFUNCTION(BlueprintCallable, Category = "SkillNode")
    void SetController(USkillTreeWidgetController* InController);
    
    // === DELEGATES ===
    
    /** Disparado quando o estado de uma habilidade muda */
    UPROPERTY(BlueprintAssignable, Category = "SkillNode|Events")
    FOnSkillStateChanged_Event OnSkillStateChanged_Event;
    
    /** Notifica mudança de estado de uma habilidade */
    UFUNCTION(BlueprintCallable, Category = "SkillNode|Events")
    void NotifySkillStateChanged(ESkillState NewState, bool bIsUnlocked, bool bIsEquipped, bool bCanUnlock);
    
protected:
    // === WIDGETS ===
    
    /** Ícone de status (verde/amarelo/cinza) */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* StatusIcon;
    
    /** Botão de desbloqueio */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* UnlockButton;
    
    // === DADOS ===
    
    /** ID do slot configurável no Editor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Node")
    FName SlotID;

    /** SkillID calculado dinamicamente baseado no personagem e slot */
    UPROPERTY()
    FName CalculatedSkillID;
    
    /** Controller da árvore */
    UPROPERTY()
    TObjectPtr<USkillTreeWidgetController> Controller;
    
    // === EVENTOS ===
    
    /** Handler para clique no botão de desbloqueio */
    UFUNCTION()
    void OnUnlockClicked();
    
private:
    /** Atualiza o estado visual do nó */
    void UpdateNodeState();
}; 