// Copyright Druid Mechanics


#include "UI/Widget/RPGSkillNodeDetailsWidget.h"

void URPGSkillNodeDetailsWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Inicialmente oculto
    SetVisibility(ESlateVisibility::Hidden);
    bIsVisible = true;
}
void URPGSkillNodeDetailsWidget::ShowDetails()
{
    SetVisibility(ESlateVisibility::Visible);
    bIsVisible = true;
}

void URPGSkillNodeDetailsWidget::HideDetails()
{
    SetVisibility(ESlateVisibility::Hidden);
    bIsVisible = false;
} 