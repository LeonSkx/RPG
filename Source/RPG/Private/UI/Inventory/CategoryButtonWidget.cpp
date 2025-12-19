#include "UI/Inventory/CategoryButtonWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"

void UCategoryButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Configurar callback do botão
	if (CategoryButton)
	{
		CategoryButton->OnClicked.RemoveAll(this);
		CategoryButton->OnClicked.AddDynamic(this, &UCategoryButtonWidget::HandleButtonClicked);
	}
}

void UCategoryButtonWidget::SetupCategory(EItemCategory InCategory, const FString& InDisplayName)
{
	Category = InCategory;
	DisplayName = InDisplayName;

	// Inicializar com texto oculto (não selecionado)
	if (CategoryText)
	{
		CategoryText->SetText(FText::FromString(DisplayName));
		CategoryText->SetVisibility(ESlateVisibility::Hidden);
	}

	// TODO: Configurar ícone da categoria baseado no tipo
	// Por enquanto, deixar vazio para ser configurado via Blueprint
	if (CategoryIcon)
	{
		// CategoryIcon será configurado via Blueprint ou sistema de ícones
	}
}

void UCategoryButtonWidget::SetSelected(bool bInSelected)
{
	bIsSelected = bInSelected;
	
	// Atualizar visibilidade do texto baseado na seleção
	if (CategoryText)
	{
		if (bIsSelected)
		{
			// Mostrar texto quando selecionado
			CategoryText->SetVisibility(ESlateVisibility::Visible);
			CategoryText->SetText(FText::FromString(DisplayName));
		}
		else
		{
			// Ocultar texto quando não selecionado
			CategoryText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	
	// Atualizar escala do container principal (1.5x quando selecionado)
	if (MainContainer)
	{
		FVector2D NewScale = bInSelected ? FVector2D(1.5f, 1.5f) : FVector2D(1.0f, 1.0f);
		MainContainer->SetRenderScale(NewScale);
		
		UE_LOG(LogTemp, Log, TEXT("CategoryButton %s: Scale changed to (%.1f, %.1f)"), 
			*DisplayName, NewScale.X, NewScale.Y);
	}
}

void UCategoryButtonWidget::SetCategoryIcon(UTexture2D* InIcon)
{
	if (CategoryIcon)
	{
		if (InIcon)
		{
			// Definir o ícone da categoria
			CategoryIcon->SetBrushFromTexture(InIcon);
			UE_LOG(LogTemp, Log, TEXT("CategoryButton %s: Icon set to %s"), 
				*DisplayName, *InIcon->GetName());
		}
		else
		{
			// Limpar ícone se nullptr
			CategoryIcon->SetBrushFromTexture(nullptr);
			UE_LOG(LogTemp, Log, TEXT("CategoryButton %s: Icon cleared"), *DisplayName);
		}
	}
}

void UCategoryButtonWidget::HandleButtonClicked()
{
	// Disparar evento de clique
	OnCategoryClicked.Broadcast(Category);
}
