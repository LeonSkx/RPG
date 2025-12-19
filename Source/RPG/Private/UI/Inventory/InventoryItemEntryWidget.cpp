#include "UI/Inventory/InventoryItemEntryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Inventory/Items/ItemDataAsset.h"

void UInventoryItemEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SelectButton)
	{
		SelectButton->OnClicked.RemoveAll(this);
		SelectButton->OnClicked.AddDynamic(this, &UInventoryItemEntryWidget::HandleClicked);
		
		// HOVER AUTO-SELECTION: Adicionar sem remover eventos existentes do BP
		// CUIDADO: Não usar RemoveAll para não interferir com eventos do Blueprint
		SelectButton->OnHovered.AddDynamic(this, &UInventoryItemEntryWidget::HandleHovered);
	}
}

void UInventoryItemEntryWidget::Setup(const FInventoryItem& InItem)
{
	CurrentItem = InItem;

	if (ItemNameText)
	{
		if (InItem.ItemData)
		{
			ItemNameText->SetText(InItem.ItemData->ItemName);
		}
		else
		{
			ItemNameText->SetText(FText::FromString(TEXT("Unknown Item")));
		}
	}

	// Quantidade (X01, X12, etc.)
	if (QuantityText)
	{
		const int32 Qty = FMath::Max(1, InItem.Quantity);
		QuantityText->SetText(FText::FromString(FString::Printf(TEXT("x%02d"), Qty)));
		QuantityText->SetVisibility(ESlateVisibility::Visible);
	}

	if (TypeIcon)
	{
		UTexture2D* IconTexture = nullptr;
		if (InItem.ItemData)
		{
			// Prioridade: TypeIcon explícito do item (usando cache)
			if (InItem.ItemData->TypeIcon.ToSoftObjectPath().IsValid())
			{
				IconTexture = GetOrLoadTexture(InItem.ItemData->TypeIcon);
			}

			// Fallback: mapa por categoria (tipos de equipamento migraram para categorias)
			if (!IconTexture)
			{
				if (const TSoftObjectPtr<UTexture2D>* FoundCat = CategoryIconMap.Find(InItem.ItemData->ItemCategory))
				{
					IconTexture = GetOrLoadTexture(*FoundCat);
				}
			}
		}

		if (IconTexture)
		{
			TypeIcon->SetBrushFromTexture(IconTexture, true);
			
			TypeIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			TypeIcon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Ao configurar, por padrão escondemos o overlay de seleção
	if (SelectedOverlayImage)
	{
		SelectedOverlayImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

// Removido: carregamento automático de ícone selecionado; controle do brush fica no BP

void UInventoryItemEntryWidget::HandleClicked()
{
	OnItemSelected.Broadcast(CurrentItem);

	// Alterna visual de seleção: apenas visibilidade; brush definido no BP
	if (SelectedOverlayImage)
	{
		SelectedOverlayImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UInventoryItemEntryWidget::HandleHovered()
{
	// HOVER AUTO-SELECTION: Simula um clique quando mouse passa por cima
	UE_LOG(LogTemp, VeryVerbose, TEXT("Item hovered: %s"), 
		CurrentItem.ItemData ? *CurrentItem.ItemData->ItemName.ToString() : TEXT("Unknown"));
	
	// Broadcast como se fosse um clique, mas sem ativar visual de seleção aqui
	OnItemSelected.Broadcast(CurrentItem);
}

void UInventoryItemEntryWidget::SetSelectedVisual(bool bSelected)
{
	if (SelectedOverlayImage)
	{
		SelectedOverlayImage->SetVisibility(bSelected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

UTexture2D* UInventoryItemEntryWidget::GetOrLoadTexture(const TSoftObjectPtr<UTexture2D>& TexturePtr)
{
	if (!TexturePtr.ToSoftObjectPath().IsValid())
	{
		return nullptr;
	}

	const FString TexturePath = TexturePtr.ToSoftObjectPath().ToString();
	
	// Verificar cache primeiro
	if (TWeakObjectPtr<UTexture2D>* CachedPtr = TextureCache.Find(TexturePath))
	{
		if (UTexture2D* CachedTexture = CachedPtr->Get())
		{
			return CachedTexture;
		}
		else
		{
			// Remover entrada inválida do cache
			TextureCache.Remove(TexturePath);
		}
	}

	// Carregar textura se não estiver em cache
	UTexture2D* LoadedTexture = nullptr;
	if (TexturePtr.IsValid())
	{
		LoadedTexture = TexturePtr.Get();
	}
	else
	{
		LoadedTexture = TexturePtr.LoadSynchronous();
	}

	// Adicionar ao cache se carregada com sucesso
	if (LoadedTexture)
	{
		TextureCache.Add(TexturePath, LoadedTexture);
	}

	return LoadedTexture;
}


