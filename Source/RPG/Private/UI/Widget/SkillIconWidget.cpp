// Copyright Druid Mechanics

#include "UI/Widget/SkillIconWidget.h"
#include "Components/Image.h"

USkillIconWidget::USkillIconWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void USkillIconWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Verificar se os componentes foram encontrados
    // if (!SkillIconImage)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("SkillIconWidget: SkillIconImage não encontrado!"));
    // }
    
    // if (!CooldownProgressBar)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("SkillIconWidget: CooldownProgressBar não encontrado!"));
    // }
    
    // if (!CooldownText)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("SkillIconWidget: CooldownText não encontrado!"));
    // }
    
    // if (!CooldownOverlay)
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("SkillIconWidget: CooldownOverlay não encontrado!"));
    // }
    
    // Inicializar o widget
    InitializeWidget();
    
    bIsInitialized = true;
    
    // UE_LOG(LogTemp, Log, TEXT("SkillIconWidget: NativeConstruct concluído. SlotID: %s"), *SlotID.ToString());
}

void USkillIconWidget::NativeDestruct()
{
    bIsInitialized = false;
    
    Super::NativeDestruct();
}

// === CONFIGURAÇÃO ===

void USkillIconWidget::SetSlotID(const FName& NewSlotID)
{
    if (SlotID != NewSlotID)
    {
        SlotID = NewSlotID;
    }
}

void USkillIconWidget::InitializeWidget()
{
    // Limpar estado inicial
    if (SkillIconImage)
    {
        SkillIconImage->SetBrushFromTexture(nullptr);
    }
}

// === ESTADO ===

bool USkillIconWidget::HasIcon() const
{
    return SkillIconImage && SkillIconImage->GetBrush().GetResourceObject();
}

void USkillIconWidget::SetSkillIcon(UTexture2D* IconTexture)
{
    if (SkillIconImage)
    {
        if (IconTexture)
        {
            SkillIconImage->SetBrushFromTexture(IconTexture);
        }
        else
        {
            // Se não há ícone, limpar
            SkillIconImage->SetBrushFromTexture(nullptr);
        }
    }
}



 