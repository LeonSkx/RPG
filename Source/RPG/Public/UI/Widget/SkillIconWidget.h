// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "SkillIconWidget.generated.h"

/**
 * Widget para exibir ícone de habilidade equipada
 * Versão simplificada do SlotSkillWidget - apenas exibe ícone
 */
UCLASS()
class RPG_API USkillIconWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    USkillIconWidget(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // === CONFIGURAÇÃO ===
    
    /** Define o ID do slot */
    UFUNCTION(BlueprintCallable, Category = "Skill Icon")
    void SetSlotID(const FName& NewSlotID);
    
    /** Define o ícone do slot */
    UFUNCTION(BlueprintCallable, Category = "Skill Icon")
    void SetSkillIcon(UTexture2D* IconTexture);
    
    

    /** Inicializa o widget */
    UFUNCTION(BlueprintCallable, Category = "Skill Icon")
    void InitializeWidget();

    // === ESTADO ===
    
    /** Verifica se tem ícone */
    UFUNCTION(BlueprintPure, Category = "Skill Icon")
    bool HasIcon() const;
    
    /** Retorna o ID do slot */
    UFUNCTION(BlueprintPure, Category = "Skill Icon")
    FName GetSlotID() const { return SlotID; }
    
    /** Retorna a referência do SkillIconImage */
    UFUNCTION(BlueprintPure, Category = "Skill Icon")
    UImage* GetSkillIconImage() const { return SkillIconImage; }
    


protected:
    // === CONFIGURAÇÃO ===
    
    /** ID único do slot (configurável no Editor) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Icon")
    FName SlotID = TEXT("Primary");

    // === WIDGETS ===
    
    /** Imagem do ícone do slot */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UImage* SkillIconImage;
    
    // === DADOS INTERNOS ===
    
    /** Se o widget foi inicializado */
    UPROPERTY()
    bool bIsInitialized = false;
    
}; 