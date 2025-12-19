#include "UI/Inventory/SubtypeButtonWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"

void USubtypeButtonWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SubtypeButton)
    {
        SubtypeButton->OnClicked.AddDynamic(this, &USubtypeButtonWidget::HandleClicked);
    }
}

void USubtypeButtonWidget::SetupSubtype(const FString& InSubtypeName, UTexture2D* InIcon)
{
    SubtypeName = InSubtypeName;
    
    if (InIcon && SubtypeIcon)
    {
        SetSubtypeIcon(InIcon);
    }
    
    // Inicialmente não selecionado
    SetSelected(false);
}

void USubtypeButtonWidget::SetSelected(bool bInSelected)
{
    bIsSelected = bInSelected;
    
    if (MainContainer)
    {
        // Efeito de escala quando selecionado (similar ao CharacterSelectorWidget)
        FVector2D Scale = bInSelected ? FVector2D(1.5f, 1.5f) : FVector2D(1.0f, 1.0f);
        MainContainer->SetRenderScale(Scale);
    }
}

void USubtypeButtonWidget::HandleClicked()
{
    OnSubtypeClicked.Broadcast(SubtypeName);
}

void USubtypeButtonWidget::SetSubtypeIcon(UTexture2D* InIcon)
{
    if (SubtypeIcon && InIcon)
    {
        SubtypeIcon->SetBrushFromTexture(InIcon);
    }
}
