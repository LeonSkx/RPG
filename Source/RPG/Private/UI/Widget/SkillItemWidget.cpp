// Copyright Druid Mechanics

#include "UI/Widget/SkillItemWidget.h"
#include "UI/Widget/SkillDropdownWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "Components/Border.h"

void USkillItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Conectar evento do botão
    if (EquipButton)
    {
        EquipButton->OnClicked.AddDynamic(this, &USkillItemWidget::OnButtonClicked);
    }
    
    // Inicializar estado
    bIsSelected = false;
    bIsEquipped = false;
    LastClickTime = 0.0;
    
    // Configurar timer para reset de seleção
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            SelectionResetTimer,
            this,
            &USkillItemWidget::CheckSelectionTimeout,
            1.0f, // Verificar a cada segundo
            true
        );
    }
    
    // Atualizar visual inicial
    UpdateVisual();
}

void USkillItemWidget::NativeDestruct()
{
    // Limpar timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SelectionResetTimer);
    }
    
    // Desconectar evento do botão
    if (EquipButton)
    {
        EquipButton->OnClicked.RemoveAll(this);
    }
    
    // Limpar referências
    ParentDropdown = nullptr;
    SkillIconTexture = nullptr;
    
    Super::NativeDestruct();
}

void USkillItemWidget::SetSkillData(const FName& InSkillID, const FText& InSkillName, UTexture2D* InSkillIcon, bool bInIsEquipped)
{
    // Verificar se os dados realmente mudaram
    bool bDataChanged = (SkillID != InSkillID || !SkillName.EqualTo(InSkillName) || SkillIconTexture != InSkillIcon || bIsEquipped != bInIsEquipped);
    
    if (!bDataChanged)
    {
        return; // Não há mudanças, evitar atualização desnecessária
    }
    
    SkillID = InSkillID;
    SkillName = InSkillName;
    SkillIconTexture = InSkillIcon;
    bIsEquipped = bInIsEquipped;
    
    // Reset seleção quando dados mudam
    bIsSelected = false;
    LastClickTime = 0.0;
    
    // Atualizar visual
    UpdateVisual();
}

void USkillItemWidget::SetParentDropdown(USkillDropdownWidget* InParentDropdown)
{
    ParentDropdown = InParentDropdown;
}

void USkillItemWidget::SetSelected(bool bInSelected)
{
    if (bIsSelected != bInSelected)
    {
        bIsSelected = bInSelected;
        LastClickTime = bInSelected ? FPlatformTime::Seconds() : 0.0;
        UpdateVisualState();
        
    }
}

void USkillItemWidget::OnEquipButtonClicked()
{
    OnButtonClicked();
}

void USkillItemWidget::UpdateVisual()
{
    UpdateSkillIcon();
    UpdateSkillName();
    UpdateVisualState();
}

void USkillItemWidget::UpdateSkillIcon()
{
    if (!SkillIcon)
    {
        return;
    }
    
    if (SkillIconTexture)
    {
        // Aplicar textura da skill
        SkillIcon->SetBrushFromTexture(SkillIconTexture);
        SkillIcon->SetColorAndOpacity(FLinearColor::White);
    }
    else
    {
        // Fallback - ícone cinza para skills sem textura
        SkillIcon->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
    }
}

void USkillItemWidget::UpdateSkillName()
{
    if (SkillNameText)
    {
        SkillNameText->SetText(SkillName);
        
        // Aplicar cor baseada no estado
        if (bIsEquipped)
        {
            SkillNameText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
        }
        else if (bIsSelected)
        {
            SkillNameText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
        }
        else
        {
            SkillNameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        }
        
    }
}

void USkillItemWidget::UpdateVisualState()
{
    // Atualizar botão
    UpdateButtonState();
    
    // Atualizar borda/background se disponível
    UpdateBackground();
    
    // Atualizar texto
    UpdateSkillName();
}

void USkillItemWidget::UpdateButtonState()
{
    if (!EquipButton)
    {
        return;
    }
    
    // Habilitar/desabilitar botão baseado no estado
    bool bShouldBeEnabled = !bIsEquipped; // Só pode equipar se não estiver equipado
    EquipButton->SetIsEnabled(bShouldBeEnabled);
    
    // Configurar estilo do botão baseado no estado
    FButtonStyle ButtonStyle = EquipButton->GetStyle();
    
    if (bIsEquipped)
    {
        // Skill já equipada - botão desabilitado/verde
        ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor::Green);
        ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor::Green);
        ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor::Green);
        ButtonStyle.Disabled.TintColor = FSlateColor(FLinearColor(0.0f, 0.5f, 0.0f));
    }
    else if (bIsSelected)
    {
        // Skill selecionada - botão amarelo (pronto para equipar)
        ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor::Yellow);
        ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(1.0f, 1.0f, 0.5f));
        ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(0.8f, 0.8f, 0.0f));
    }
    else
    {
        // Estado normal - botão azul/padrão
        ButtonStyle.Normal.TintColor = FSlateColor(FLinearColor(0.3f, 0.3f, 1.0f));
        ButtonStyle.Hovered.TintColor = FSlateColor(FLinearColor(0.5f, 0.5f, 1.0f));
        ButtonStyle.Pressed.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.8f));
    }
    
    EquipButton->SetStyle(ButtonStyle);
}

void USkillItemWidget::UpdateBackground()
{
    // Se há um componente Border ou Background, atualizar cor
    if (UBorder* BackgroundBorder = Cast<UBorder>(GetWidgetFromName(TEXT("BackgroundBorder"))))
    {
        FLinearColor BackgroundColor;
        
        if (bIsEquipped)
        {
            BackgroundColor = FLinearColor(0.0f, 0.3f, 0.0f, 0.5f); // Verde transparente
        }
        else if (bIsSelected)
        {
            BackgroundColor = FLinearColor(0.3f, 0.3f, 0.0f, 0.5f); // Amarelo transparente
        }
        else
        {
            BackgroundColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.3f); // Cinza transparente
        }
        
        BackgroundBorder->SetBrushColor(BackgroundColor);
    }
}

void USkillItemWidget::OnButtonClicked()
{
    double CurrentTime = FPlatformTime::Seconds();
    
    // Se já está equipado, não fazer nada
    if (bIsEquipped)
    {
        return;
    }
    
    // Se não está selecionado, ou se passou tempo demais desde a última seleção
    if (!bIsSelected || (CurrentTime - LastClickTime > 3.0f))
    {
        // Primeiro clique ou timeout - selecionar
        SetSelected(true);
        
        // Notificar dropdown para desselecionar outros itens
        if (ParentDropdown)
        {
            ParentDropdown->OnSkillItemSelected(this);
        }
    }
    else
    {
        // Segundo clique dentro do tempo limite - confirmar e equipar
        double TimeSinceLastClick = CurrentTime - LastClickTime;
        
        if (TimeSinceLastClick < 3.0f) // 3 segundos para confirmar
        {
            if (ParentDropdown)
            {
                ParentDropdown->EquipSkill(SkillID);
            }
        }
        else
        {
            // Timeout - tratar como primeiro clique novamente
            SetSelected(true);
        }
    }
}

void USkillItemWidget::CheckSelectionTimeout()
{
    if (bIsSelected)
    {
        double CurrentTime = FPlatformTime::Seconds();
        double TimeSinceSelection = CurrentTime - LastClickTime;
        
        // Se passou mais de 5 segundos desde a seleção, resetar
        if (TimeSinceSelection > 5.0f)
        {
            SetSelected(false);
        }
    }
}

bool USkillItemWidget::IsSelected() const
{
    return bIsSelected;
}

bool USkillItemWidget::IsEquipped() const
{
    return bIsEquipped;
}

FName USkillItemWidget::GetSkillID() const
{
    return SkillID;
}

FText USkillItemWidget::GetSkillName() const
{
    return SkillName;
} 