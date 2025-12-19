// Copyright Druid Mechanics

#include "UI/Widget/SlotSkillWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

USlotSkillWidget::USlotSkillWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , SlotID(TEXT("Primary"))
    , EquippedSkillID(NAME_None)
    , bIsInitialized(false)
{
}

void USlotSkillWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Configurar eventos dos botões
    SetupButtonEvents();
    
    // Marcar como inicializado
    bIsInitialized = true;
}

void USlotSkillWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

// === CONFIGURAÇÃO ===

void USlotSkillWidget::SetSlotID(const FName& NewSlotID)
{
    SlotID = NewSlotID;
}

void USlotSkillWidget::SetSlotIcon(UTexture2D* IconTexture)
{
    if (SlotIconImage && IconTexture)
    {
        SlotIconImage->SetBrushFromTexture(IconTexture);
    }
}

void USlotSkillWidget::InitializeSlot()
{
    // Configurar eventos dos botões
    SetupButtonEvents();
    
    // Marcar como inicializado
    bIsInitialized = true;
}

// === ESTADO ===

void USlotSkillWidget::SetEquippedSkill(const FName& SkillID)
{
    EquippedSkillID = SkillID;
    
    // Aqui você pode implementar lógica adicional quando uma habilidade é equipada
    // Por exemplo: atualizar ícone, mostrar nome da habilidade, etc.
}

void USlotSkillWidget::ClearSlot()
{
    EquippedSkillID = NAME_None;
    
    // Aqui você pode implementar lógica adicional quando o slot é limpo
    // Por exemplo: restaurar ícone padrão, limpar textos, etc.
}

bool USlotSkillWidget::IsEmpty() const
{
    return EquippedSkillID.IsNone();
}

bool USlotSkillWidget::IsEquipped() const
{
    return !EquippedSkillID.IsNone();
}

FName USlotSkillWidget::GetEquippedSkillID() const
{
    return EquippedSkillID;
}

// === TESTE DE EVENTOS ===

void USlotSkillWidget::TestSlotClicked()
{
    // Disparar delegate de clique
    OnSkillSlotClicked.Broadcast(SlotID);
}

// === HELPERS ===

void USlotSkillWidget::SetupButtonEvents()
{
    // Aqui você pode implementar a configuração de eventos dos botões
    // Por exemplo: conectar OnClicked, OnRightClicked, etc.
}

void USlotSkillWidget::OnSlotButtonClicked()
{
    // Disparar delegate de clique
    OnSkillSlotClicked.Broadcast(SlotID);
}
