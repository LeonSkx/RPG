// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "GameplayTagContainer.h"
#include "SlotSkillWidget.generated.h"

class USkillEquipmentComponent;

// Delegate para eventos do slot
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlotEquipped, FName, SlotID, FName, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSlotUnequipped, FName, SlotID, FName, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillSlotClicked, FName, SlotID);

/**
 * Widget para representar um slot individual de habilidade
 * Gerencia equipamento/desequipamento e integração com SkillEquipmentComponent
 */
UCLASS()
class RPG_API USlotSkillWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    USlotSkillWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // === CONFIGURAÇÃO ===
    
    /** Define o ID do slot */
    UFUNCTION(BlueprintCallable, Category = "Slot Skill")
    void SetSlotID(const FName& NewSlotID);
    
    /** Define o ícone do slot */
    UFUNCTION(BlueprintCallable, Category = "Slot Skill")
    void SetSlotIcon(UTexture2D* IconTexture);
    
    /** Inicializa o slot */
    UFUNCTION(BlueprintCallable, Category = "Slot Skill")
    void InitializeSlot();
    
    // === ESTADO ===
    
    /** Define a habilidade equipada no slot */
    UFUNCTION(BlueprintCallable, Category = "Slot Skill")
    void SetEquippedSkill(const FName& SkillID);
    
    /** Limpa o slot (remove habilidade) */
    UFUNCTION(BlueprintCallable, Category = "Slot Skill")
    void ClearSlot();
    
    /** Verifica se o slot está vazio */
    UFUNCTION(BlueprintPure, Category = "Slot Skill")
    bool IsEmpty() const;
    
    /** Verifica se o slot tem uma habilidade equipada */
    UFUNCTION(BlueprintPure, Category = "Slot Skill")
    bool IsEquipped() const;
    
    /** Retorna o ID da habilidade equipada */
    UFUNCTION(BlueprintPure, Category = "Slot Skill")
    FName GetEquippedSkillID() const;
    
    /** Retorna o ID do slot */
    UFUNCTION(BlueprintPure, Category = "Slot Skill")
    FName GetSlotID() const { return SlotID; }

    /** Retorna a InputTag do slot */
    UFUNCTION(BlueprintPure, Category = "Slot Skill")
    FGameplayTag GetInputTag() const { return InputTag; }
    
    /** Retorna a referência do SlotIconImage */
    UFUNCTION(BlueprintPure, Category = "Slot Skill")
    UImage* GetSlotIconImage() const { return SlotIconImage; }

    // === TESTE DE EVENTOS ===
    
    /** Testa o clique do slot */
    UFUNCTION(BlueprintCallable, Category = "Slot Skill|Test")
    void TestSlotClicked();

    // === DELEGATES ===
    
    /** Disparado quando o slot é clicado */
    UPROPERTY(BlueprintAssignable, Category = "Slot Skill|Events")
    FOnSkillSlotClicked OnSkillSlotClicked;

protected:
    // === CONFIGURAÇÃO ===
    
    /** ID único do slot (configurável no Editor) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Skill")
    FName SlotID = TEXT("Primary");

    /** Tag de input do slot (configurável no Editor) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Skill")
    FGameplayTag InputTag;

    // === WIDGETS ===
    
    /** Imagem do ícone do slot */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* SlotIconImage;

    // === DADOS INTERNOS ===
    
    /** ID da habilidade equipada */
    UPROPERTY(BlueprintReadOnly, Category = "Slot Skill")
    FName EquippedSkillID = NAME_None;
    
    /** Se o slot foi inicializado */
    UPROPERTY()
    bool bIsInitialized = false;

protected:
    // === HELPERS ===
    void SetupButtonEvents();
    
    UFUNCTION()
    void OnSlotButtonClicked();
};
