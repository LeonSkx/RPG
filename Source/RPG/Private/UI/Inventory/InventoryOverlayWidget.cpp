#include "UI/Inventory/InventoryOverlayWidget.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/HorizontalBox.h"

#include "Components/ScrollBox.h"
#include "UI/Inventory/CategoryButtonWidget.h"
#include "UI/Inventory/SubtypeButtonWidget.h"
#include "UI/Inventory/InventoryItemEntryWidget.h"
#include "Inventory/Core/InventorySubsystem.h"
#include "Inventory/Core/InventoryEnums.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "UI/Equipment/ItemsDetailsOverlayWidget.h"
#include "Inventory/Data/CategoryIconsDataAsset.h"
#include "UI/UIUtilities.h"

void UInventoryOverlayWidget::NativeConstruct()
{
	Super::NativeConstruct();
    
    InitializeComponents();
}

void UInventoryOverlayWidget::InitializeComponents()
{
    // Título básico
    if (TitleText)
        TitleText->SetText(FText::FromString(TEXT("Inventory")));

    // Criar botões de categoria
    CreateCategoryButtons();
}



void UInventoryOverlayWidget::SetTitle(const FText& InTitle)
{
    if (TitleText)
    {
        TitleText->SetText(InTitle);
    }
}

void UInventoryOverlayWidget::OnItemSelected(const FInventoryItem& SelectedItem)
{
    // Desativar auto-hover inicial se estiver ativo
    if (bFirstItemAutoHover)
    {
        bFirstItemAutoHover = false;
        UE_LOG(LogTemp, Log, TEXT("InventoryOverlayWidget: Auto-hover inicial desativado pelo usuário"));
    }
    
    // TODO: Implementar sistema de marcação de itens vistos quando InventorySubsystem estiver funcionando
    // Por enquanto, itens sempre aparecem como "novos"
    if (SelectedItem.bIsNew)
    {
        UE_LOG(LogTemp, Log, TEXT("Item novo selecionado: %s (sistema de marcação desabilitado temporariamente)"), 
            SelectedItem.ItemData ? *SelectedItem.ItemData->ItemName.ToString() : TEXT("Unknown"));
    }
    
    // IMPORTANTE: Atualizar seleção visual de forma mais robusta
    // Primeiro, desativar o item anterior se for diferente
    if (ActiveHoverItem && ActiveHoverItem->GetCurrentItem().ItemData != SelectedItem.ItemData)
    {
        ActiveHoverItem->SetSelectedVisual(false);
        ActiveHoverItem = nullptr;
    }
    
    // Encontrar e ativar o novo item
    for (TWeakObjectPtr<UInventoryItemEntryWidget>& EntryPtr : ItemEntries)
    {
        if (UInventoryItemEntryWidget* EntryWidget = EntryPtr.Get())
        {
            if (EntryWidget->GetCurrentItem().ItemData == SelectedItem.ItemData)
            {
                // Ativar seleção visual
                EntryWidget->SetSelectedVisual(true);
                ActiveHoverItem = EntryWidget;
                
                // Log para debug
                UE_LOG(LogTemp, Log, TEXT("InventoryOverlayWidget: Seleção ativada em %s"), 
                    *SelectedItem.ItemData->ItemName.ToString());
                break;
            }
        }
    }
    
    ApplyItemSelection(SelectedItem);
}



void UInventoryOverlayWidget::ApplyItemSelection(const FInventoryItem& SelectedItem)
{
    // Criar widget de detalhes sob demanda
    if (!ItemDetailsWidget && ItemDetailsClass && DetailsContainer)
    {
        ItemDetailsWidget = CreateWidget<UItemsDetailsOverlayWidget>(this, ItemDetailsClass);
        if (ItemDetailsWidget)
            DetailsContainer->SetContent(ItemDetailsWidget);
    }
    
    if (ItemDetailsWidget)
        ItemDetailsWidget->Setup(SelectedItem);
    
    // TODO: Implementar seleção visual nos entries quando necessário
    UE_LOG(LogTemp, Log, TEXT("Item selected: %s"), 
        SelectedItem.ItemData ? *SelectedItem.ItemData->ItemName.ToString() : TEXT("Unknown"));
}

UInventorySubsystem* UInventoryOverlayWidget::GetInventorySubsystem() const
{
    if (const UWorld* World = GetWorld())
        if (UGameInstance* GameInstance = World->GetGameInstance())
            return GameInstance->GetSubsystem<UInventorySubsystem>();
    return nullptr;
}

void UInventoryOverlayWidget::CreateCategoryButtons()
{
    if (!CategoryButtonsContainer || !CategoryButtonClass)
        return;

    // Limpar botões existentes
    CategoryButtonsContainer->ClearChildren();

    // Array de categorias para criar botões (excluindo None)
    TArray<EItemCategory> CategoriesToCreate = {
        EItemCategory::New,        // Categoria especial para itens novos
        EItemCategory::Weapon,
        EItemCategory::Armor,
        EItemCategory::Boots,
        EItemCategory::Accessory,
        EItemCategory::Ring,
        EItemCategory::Consumable,
        EItemCategory::Material,
        EItemCategory::Valuable,
        EItemCategory::Cosmetic,
        EItemCategory::Expansion
    };

    // Criar botão para cada categoria
    for (EItemCategory Category : CategoriesToCreate)
    {
        UCategoryButtonWidget* CategoryButton = CreateWidget<UCategoryButtonWidget>(this, CategoryButtonClass);
        if (CategoryButton)
        {
            // Obter nome de exibição da categoria
            FString DisplayName = GetCategoryDisplayName(Category);
            
            // Configurar o botão
            CategoryButton->SetupCategory(Category, DisplayName);
            
            // Configurar ícone da categoria
            if (CategoryIconsData)
            {
                UTexture2D* CategoryIcon = nullptr;
                
                // Caso especial: categoria "New" usa NewIcon
                if (Category == EItemCategory::New)
                {
                    CategoryIcon = CategoryIconsData->GetNewIcon();
                }
                else
                {
                    CategoryIcon = CategoryIconsData->GetIconForCategory(Category);
                }
                
                CategoryButton->SetCategoryIcon(CategoryIcon);
                
                UE_LOG(LogTemp, Log, TEXT("Set icon for category %s: %s"), 
                    *DisplayName,
                    CategoryIcon ? *CategoryIcon->GetName() : TEXT("NULL"));
            }
            
            // Conectar evento de clique
            CategoryButton->OnCategoryClicked.AddDynamic(this, &UInventoryOverlayWidget::OnCategoryClicked);
            
            // Adicionar ao container
            CategoryButtonsContainer->AddChild(CategoryButton);
        }
    }
    
    // Auto-hover na primeira categoria (mais robusto)
    if (CategoriesToCreate.Num() > 0)
    {
        bFirstCategoryAutoHover = true;
        
        // IMPORTANTE: Selecionar automaticamente a primeira categoria
        EItemCategory FirstCategory = CategoriesToCreate[0];
        ApplyCategoryFilter(FirstCategory);
        
        // Log para debug
        UE_LOG(LogTemp, Log, TEXT("InventoryOverlayWidget: Auto-hover inicial ativado na primeira categoria: %s"), 
            *GetCategoryDisplayName(FirstCategory));
    }
}

void UInventoryOverlayWidget::CreateSubtypeButtons(EItemCategory Category)
{
    if (!SubtypeButtonsContainer || !SubtypeButtonClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("SubtypeButtonsContainer or SubtypeButtonClass not found"));
        return;
    }
    
    // Limpar botões existentes
    ClearSubtypeButtons();
    
    // Obter subtypes disponíveis para a categoria
    TArray<FString> AvailableSubtypes = GetAvailableSubtypes(Category);
    
    if (AvailableSubtypes.Num() > 0)
    {
        // Configurar o container para preencher da direita para a esquerda
        // Isso faz com que os botões sejam posicionados da direita para a esquerda
        SubtypeButtonsContainer->SetFlowDirectionPreference(EFlowDirectionPreference::LeftToRight);
        // Botão "All" para mostrar todos os itens da categoria
        USubtypeButtonWidget* AllButton = CreateWidget<USubtypeButtonWidget>(this, SubtypeButtonClass);
        if (AllButton)
        {
            AllButton->SetupSubtype(TEXT("All"));
            AllButton->OnSubtypeClicked.AddDynamic(this, &UInventoryOverlayWidget::OnSubtypeClicked);
            
            // Configurar ícone "All" (usado em todas as categorias)
            if (CategoryIconsData)
            {
                UTexture2D* AllIcon = CategoryIconsData->GetAllIcon();
                if (AllIcon)
                {
                    AllButton->SetSubtypeIcon(AllIcon);
                    UE_LOG(LogTemp, Log, TEXT("CreateSubtypeButtons: Set All icon for category %s: %s"), 
                        *GetCategoryDisplayName(Category), *AllIcon->GetName());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("CreateSubtypeButtons: No All icon available"));
                }
            }
            
            SubtypeButtonsContainer->AddChild(AllButton);
            SubtypeButtons.Add(AllButton);
            
            // Auto-selecionar o botão "All"
            AllButton->SetSelected(true);
            CurrentSelectedSubtype = TEXT("All");
            
            // Atualizar o texto inicial para "All Items"
            if (SubtypeNameText)
            {
                SubtypeNameText->SetText(FText::FromString(TEXT("All Items")));
            }
        }
        
        // Botões para cada subtype disponível
        for (const FString& SubtypeName : AvailableSubtypes)
        {
            USubtypeButtonWidget* SubtypeButton = CreateWidget<USubtypeButtonWidget>(this, SubtypeButtonClass);
            if (SubtypeButton)
            {
                // Configurar o botão com o subtype
                SubtypeButton->SetupSubtype(SubtypeName);
                
                // Configurar ícone de forma robusta
                UTexture2D* SubtypeIcon = GetSubtypeIcon(Category, SubtypeName);
                if (SubtypeIcon)
                {
                    // Configurar o ícone de forma segura
                    SubtypeButton->SetSubtypeIcon(SubtypeIcon);
                    UE_LOG(LogTemp, Log, TEXT("CreateSubtypeButtons: Successfully set icon for subtype %s: %s"), 
                        *SubtypeName, *SubtypeIcon->GetName());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("CreateSubtypeButtons: No icon available for subtype %s"), *SubtypeName);
                }
                
                SubtypeButton->OnSubtypeClicked.AddDynamic(this, &UInventoryOverlayWidget::OnSubtypeClicked);
                SubtypeButtonsContainer->AddChild(SubtypeButton);
                SubtypeButtons.Add(SubtypeButton);
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Created %d subtype buttons for category %s"), 
            SubtypeButtons.Num(), *GetCategoryDisplayName(Category));
    }
}

TArray<FString> UInventoryOverlayWidget::GetAvailableSubtypes(EItemCategory Category)
{
    TArray<FString> AllSubtypes;
    TArray<FString> AvailableSubtypes;
    
    // Obter todos os subtypes possíveis para a categoria
    switch (Category)
    {
        case EItemCategory::New:
            // Categoria especial: não tem subtypes, retorna vazio
            return AvailableSubtypes;
            
        case EItemCategory::Weapon:
            AllSubtypes = UIUtilities::GetEnumValues<EWeaponType>();
            break;
            
        case EItemCategory::Armor:
            AllSubtypes = UIUtilities::GetEnumValues<EArmorType>();
            break;
            
        case EItemCategory::Boots:
            AllSubtypes = UIUtilities::GetEnumValues<EBootsType>();
            break;
            
        case EItemCategory::Accessory:
            AllSubtypes = UIUtilities::GetEnumValues<EAccessoryType>();
            break;
            
        case EItemCategory::Ring:
            AllSubtypes = UIUtilities::GetEnumValues<ERingType>();
            break;
            
        case EItemCategory::Consumable:
            AllSubtypes = UIUtilities::GetEnumValues<EConsumableType>();
            break;
            
        case EItemCategory::Material:
        case EItemCategory::Valuable:
        case EItemCategory::Cosmetic:
        case EItemCategory::Expansion:
            // Essas categorias não têm subtypes específicos
            return AvailableSubtypes;
            
        default:
            return AvailableSubtypes;
    }
    
    // Verificar quais subtypes realmente existem no inventário
    UInventorySubsystem* InventorySystem = GetInventorySubsystem();
    if (!InventorySystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem not found"));
        return AvailableSubtypes;
    }
    
    TArray<FInventoryItem> AllItems = InventorySystem->GetAllItems();
    
    for (const FString& SubtypeName : AllSubtypes)
    {
        bool bHasItemsOfThisSubtype = false;
        
        for (const FInventoryItem& Item : AllItems)
        {
            if (Item.ItemData && Item.ItemData->ItemCategory == Category)
            {
                // Verificar se o item é deste subtype
                bool bMatchesSubtype = false;
                
                switch (Category)
                {
                    case EItemCategory::Weapon:
                        bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->WeaponType) == SubtypeName);
                        break;
                        
                    case EItemCategory::Armor:
                        bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->ArmorType) == SubtypeName);
                        break;
                        
                    case EItemCategory::Boots:
                        bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->BootsType) == SubtypeName);
                        break;
                        
                    case EItemCategory::Accessory:
                        bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->AccessoryType) == SubtypeName);
                        break;
                        
                    case EItemCategory::Ring:
                        bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->RingType) == SubtypeName);
                        break;
                        
                    case EItemCategory::Consumable:
                        bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->ConsumableType) == SubtypeName);
                        break;
                        
                    default:
                        break;
                }
                
                if (bMatchesSubtype)
                {
                    bHasItemsOfThisSubtype = true;
                    break; // Encontrou pelo menos um item deste subtype
                }
            }
        }
        
        // Só adicionar se houver itens deste subtype
        if (bHasItemsOfThisSubtype)
        {
            AvailableSubtypes.Add(SubtypeName);
            UE_LOG(LogTemp, Log, TEXT("Subtype %s has items in inventory"), *SubtypeName);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Subtype %s has NO items in inventory - skipping"), *SubtypeName);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Found %d available subtypes (out of %d total) for category %s"), 
        AvailableSubtypes.Num(), AllSubtypes.Num(), *GetCategoryDisplayName(Category));
    
    return AvailableSubtypes;
}

UTexture2D* UInventoryOverlayWidget::GetSubtypeIcon(EItemCategory Category, const FString& SubtypeName)
{
    // Validações básicas
    if (SubtypeName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("GetSubtypeIcon: SubtypeName is empty"));
        return nullptr;
    }
    
    UInventorySubsystem* InventorySystem = GetInventorySubsystem();
    if (!InventorySystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetSubtypeIcon: InventorySubsystem not found"));
        return nullptr;
    }
    
    TArray<FInventoryItem> AllItems = InventorySystem->GetAllItems();
    if (AllItems.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("GetSubtypeIcon: No items found in inventory"));
        return nullptr;
    }
    
    // Procurar pelo primeiro item deste subtype para pegar o TypeIcon
    for (const FInventoryItem& Item : AllItems)
    {
        // Validação robusta do item
        if (!Item.ItemData)
        {
            UE_LOG(LogTemp, VeryVerbose, TEXT("GetSubtypeIcon: Item has no ItemData, skipping"));
            continue;
        }
        
        if (Item.ItemData->ItemCategory != Category)
        {
            continue; // Item não é da categoria desejada
        }
        
        // Verificar se o item é deste subtype
        bool bMatchesSubtype = false;
        
        switch (Category)
        {
            case EItemCategory::Weapon:
                if (Item.ItemData->WeaponType != EWeaponType::None)
                {
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->WeaponType) == SubtypeName);
                }
                break;
                
            case EItemCategory::Armor:
                if (Item.ItemData->ArmorType != EArmorType::None)
                {
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->ArmorType) == SubtypeName);
                }
                break;
                
            case EItemCategory::Boots:
                if (Item.ItemData->BootsType != EBootsType::None)
                {
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->BootsType) == SubtypeName);
                }
                break;
                
            case EItemCategory::Accessory:
                if (Item.ItemData->AccessoryType != EAccessoryType::None)
                {
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->AccessoryType) == SubtypeName);
                }
                break;
                
            case EItemCategory::Ring:
                if (Item.ItemData->RingType != ERingType::None)
                {
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->RingType) == SubtypeName);
                }
                break;
                
            case EItemCategory::Consumable:
                if (Item.ItemData->ConsumableType != EConsumableType::None)
                {
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->ConsumableType) == SubtypeName);
                }
                break;
                
            default:
                UE_LOG(LogTemp, Warning, TEXT("GetSubtypeIcon: Unsupported category %d"), static_cast<int32>(Category));
                continue;
        }
        
        if (bMatchesSubtype)
        {
            // Verificar se o item tem TypeIcon válido
            if (!Item.ItemData->TypeIcon.ToSoftObjectPath().IsValid())
            {
                UE_LOG(LogTemp, VeryVerbose, TEXT("GetSubtypeIcon: Item %s has no valid TypeIcon path"), 
                    *Item.ItemData->ItemName.ToString());
                continue;
            }
            
            // Carregar o ícone de forma segura
            UTexture2D* IconTexture = Item.ItemData->TypeIcon.LoadSynchronous();
            
            if (IconTexture)
            {
                UE_LOG(LogTemp, Log, TEXT("GetSubtypeIcon: Successfully found icon for subtype %s from item %s: %s"), 
                    *SubtypeName, *Item.ItemData->ItemName.ToString(), *IconTexture->GetName());
                return IconTexture;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("GetSubtypeIcon: Failed to load icon for item %s"), 
                    *Item.ItemData->ItemName.ToString());
                continue;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("GetSubtypeIcon: No icon found for subtype %s in category %s"), 
        *SubtypeName, *GetCategoryDisplayName(Category));
    return nullptr;
}

void UInventoryOverlayWidget::ClearSubtypeButtons()
{
    if (!SubtypeButtonsContainer)
    {
        return;
    }
    
    // Limpar referências aos botões
    for (TWeakObjectPtr<USubtypeButtonWidget>& ButtonPtr : SubtypeButtons)
    {
        if (USubtypeButtonWidget* Button = ButtonPtr.Get())
        {
            Button->RemoveFromParent();
        }
    }
    
    // Limpar arrays
    SubtypeButtons.Empty();
    SubtypeButtonsContainer->ClearChildren();
    
    // Limpar o texto do subtype
    if (SubtypeNameText)
    {
        SubtypeNameText->SetText(FText::FromString(TEXT("")));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Cleared subtype buttons"));
}

FString UInventoryOverlayWidget::GetCategoryDisplayName(EItemCategory Category)
{
    // Mapeamento de nomes de exibição para localização futura
    static TMap<EItemCategory, FString> CategoryNames = {
        {EItemCategory::New, "New"},
        {EItemCategory::Weapon, "Weapon"},
        {EItemCategory::Armor, "Armor"},
        {EItemCategory::Boots, "Boots"},
        {EItemCategory::Accessory, "Accessory"},
        {EItemCategory::Ring, "Ring"},
        {EItemCategory::Consumable, "Consumable"},
        {EItemCategory::Material, "Material"},
        {EItemCategory::Valuable, "Valuable"},
        {EItemCategory::Cosmetic, "Cosmetic"},
        {EItemCategory::Expansion, "Expansion"}
    };

    FString* NamePtr = CategoryNames.Find(Category);
    return NamePtr ? *NamePtr : "Unknown";
}

void UInventoryOverlayWidget::OnCategoryClicked(EItemCategory Category)
{
    // Desativar auto-hover inicial se estiver ativo
    if (bFirstCategoryAutoHover)
    {
        bFirstCategoryAutoHover = false;
        UE_LOG(LogTemp, Log, TEXT("InventoryOverlayWidget: Auto-hover inicial desativado pelo usuário"));
    }
    
    // Aplicar filtro por categoria
    ApplyCategoryFilter(Category);
    
    // Log para debug
    UE_LOG(LogTemp, Log, TEXT("Category clicked: %d - %s"), 
        static_cast<int32>(Category), 
        *GetCategoryDisplayName(Category));
}

void UInventoryOverlayWidget::OnSubtypeClicked(const FString& SubtypeName)
{
    // Aplicar filtro por subtype
    ApplySubtypeFilter(SubtypeName);
    
    UE_LOG(LogTemp, Log, TEXT("Subtype clicked: %s"), *SubtypeName);
}

void UInventoryOverlayWidget::ApplyCategoryFilter(EItemCategory Category)
{
    // Atualizar categoria selecionada
    CurrentSelectedCategory = Category;
    
    // Criar botões de subtype para a categoria selecionada
    CreateSubtypeButtons(Category);
    
    // Popular lista de itens com a categoria selecionada
    PopulateItemsList(Category);
    
    // Obter e exibir itens da categoria selecionada (para debug)
    DisplayItemsByCategory(Category);
    
    // Atualizar feedback visual dos botões
    UpdateCategoryButtonSelection(Category);
    
    UE_LOG(LogTemp, Log, TEXT("Applied category filter: %s"), 
        *GetCategoryDisplayName(Category));
}

void UInventoryOverlayWidget::ApplySubtypeFilter(const FString& SubtypeName)
{
    // Atualizar subtype selecionado
    CurrentSelectedSubtype = SubtypeName;
    
    // Popular lista de itens com o subtype selecionado
    if (SubtypeName == TEXT("All"))
    {
        PopulateItemsList(CurrentSelectedCategory);
    }
    else
    {
        PopulateItemsListBySubtype(SubtypeName);
    }
    
    // Atualizar feedback visual dos botões de subtype
    UpdateSubtypeButtonSelection(SubtypeName);
    
    UE_LOG(LogTemp, Log, TEXT("Applied subtype filter: %s"), *SubtypeName);
}

EInventoryFilterCategory UInventoryOverlayWidget::ConvertItemCategoryToFilterCategory(EItemCategory ItemCategory)
{
    // Mapeamento de EItemCategory para EInventoryFilterCategory
    switch (ItemCategory)
    {
        case EItemCategory::Weapon:      return EInventoryFilterCategory::Weapon;
        case EItemCategory::Armor:       return EInventoryFilterCategory::Armor;
        case EItemCategory::Boots:       return EInventoryFilterCategory::Boots;
        case EItemCategory::Accessory:   return EInventoryFilterCategory::Accessory;
        case EItemCategory::Ring:        return EInventoryFilterCategory::Ring;
        case EItemCategory::Consumable:  return EInventoryFilterCategory::Consumable;
        case EItemCategory::Material:    return EInventoryFilterCategory::Materials;
        case EItemCategory::Valuable:    return EInventoryFilterCategory::Quest; // Quest é usado para Valuable
        case EItemCategory::Cosmetic:    return EInventoryFilterCategory::Cosmetic;
        case EItemCategory::Expansion:   return EInventoryFilterCategory::Expansion;
        default:                         return EInventoryFilterCategory::None;
    }
}

void UInventoryOverlayWidget::DisplayItemsByCategory(EItemCategory Category)
{
    UInventorySubsystem* InventorySystem = GetInventorySubsystem();
    if (!InventorySystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem not found"));
        return;
    }
    
    // Obter todos os itens do inventário
    TArray<FInventoryItem> AllItems = InventorySystem->GetAllItems();
    
    // Filtrar itens pela categoria
    TArray<FInventoryItem> FilteredItems;
    for (const FInventoryItem& Item : AllItems)
    {
        if (Item.ItemData && Item.ItemData->ItemCategory == Category)
        {
            FilteredItems.Add(Item);
        }
    }
    
    // Log dos itens encontrados
    UE_LOG(LogTemp, Log, TEXT("Found %d items for category %s:"), 
        FilteredItems.Num(), 
        *GetCategoryDisplayName(Category));
    
    for (const FInventoryItem& Item : FilteredItems)
    {
        if (Item.ItemData)
        {
            UE_LOG(LogTemp, Log, TEXT("  - %s"), 
                *Item.ItemData->ItemName.ToString());
        }
    }
    
    // TODO: Atualizar a lista visual com os itens filtrados
    // Por enquanto, só logamos para debug
}

void UInventoryOverlayWidget::UpdateCategoryButtonSelection(EItemCategory SelectedCategory)
{
    if (!CategoryButtonsContainer)
    {
        return;
    }
    
    // Percorrer todos os botões de categoria
    for (int32 i = 0; i < CategoryButtonsContainer->GetChildrenCount(); i++)
    {
        UWidget* ChildWidget = CategoryButtonsContainer->GetChildAt(i);
        UCategoryButtonWidget* CategoryButton = Cast<UCategoryButtonWidget>(ChildWidget);
        
        if (CategoryButton)
        {
            // Verificar se este botão representa a categoria selecionada
            bool bShouldBeSelected = (CategoryButton->GetCategory() == SelectedCategory);
            
            // Aplicar estado de seleção
            CategoryButton->SetSelected(bShouldBeSelected);
            
            UE_LOG(LogTemp, Log, TEXT("Button %s: %s"), 
                *GetCategoryDisplayName(CategoryButton->GetCategory()),
                bShouldBeSelected ? TEXT("SELECTED") : TEXT("NOT SELECTED"));
        }
    }
}

void UInventoryOverlayWidget::UpdateSubtypeButtonSelection(const FString& SelectedSubtype)
{
    if (!SubtypeButtonsContainer)
    {
        return;
    }
    
    // Atualizar o texto do subtype selecionado
    if (SubtypeNameText)
    {
        if (SelectedSubtype == TEXT("All"))
        {
            SubtypeNameText->SetText(FText::FromString(TEXT("All Items")));
        }
        else
        {
            SubtypeNameText->SetText(FText::FromString(SelectedSubtype));
        }
    }
    
    // Percorrer todos os botões de subtype
    for (int32 i = 0; i < SubtypeButtonsContainer->GetChildrenCount(); i++)
    {
        UWidget* ChildWidget = SubtypeButtonsContainer->GetChildAt(i);
        USubtypeButtonWidget* SubtypeButton = Cast<USubtypeButtonWidget>(ChildWidget);
        
        if (SubtypeButton)
        {
            // Verificar se este botão representa o subtype selecionado
            bool bShouldBeSelected = (SubtypeButton->GetSubtypeName() == SelectedSubtype);
            
            // Aplicar estado de seleção
            SubtypeButton->SetSelected(bShouldBeSelected);
            
            UE_LOG(LogTemp, Log, TEXT("Subtype Button %s: %s"), 
                *SubtypeButton->GetSubtypeName(),
                bShouldBeSelected ? TEXT("SELECTED") : TEXT("NOT SELECTED"));
        }
    }
}

void UInventoryOverlayWidget::ResetAutoHover()
{
    bFirstCategoryAutoHover = false;
    UE_LOG(LogTemp, Log, TEXT("InventoryOverlayWidget: Auto-hover resetado"));
}

void UInventoryOverlayWidget::PopulateItemsList(EItemCategory Category)
{
    if (!ItemsScrollBox || !ItemEntryClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ItemsScrollBox or ItemEntryClass not found"));
        return;
    }
    
    // Limpar lista existente
    ClearItemsList();
    
    // Obter itens do inventário
    UInventorySubsystem* InventorySystem = GetInventorySubsystem();
    if (!InventorySystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem not found"));
        return;
    }
    
    // Obter todos os itens e filtrar por categoria
    TArray<FInventoryItem> AllItems = InventorySystem->GetAllItems();
    TArray<FInventoryItem> FilteredItems;
    
    if (Category == EItemCategory::New)
    {
        // TODO: Implementar sistema completo de itens novos quando InventorySubsystem estiver funcionando
        // Por enquanto, mostra apenas itens com flag bIsNew = true
        for (const FInventoryItem& Item : AllItems)
        {
            if (Item.bIsNew)
            {
                FilteredItems.Add(Item);
            }
        }
        UE_LOG(LogTemp, Log, TEXT("Found %d new items (sistema básico)"), FilteredItems.Num());
    }
    else
    {
        // Para outras categorias, filtrar por categoria normal
        for (const FInventoryItem& Item : AllItems)
        {
            if (Item.ItemData && Item.ItemData->ItemCategory == Category)
            {
                FilteredItems.Add(Item);
            }
        }
    }
    
    // Criar entries para cada item filtrado
    for (const FInventoryItem& Item : FilteredItems)
    {
        UInventoryItemEntryWidget* EntryWidget = CreateWidget<UInventoryItemEntryWidget>(this, ItemEntryClass);
        if (EntryWidget)
        {
            // Configurar o entry com o item
            EntryWidget->Setup(Item);
            
            // Vincular evento de seleção
            EntryWidget->OnItemSelected.AddDynamic(this, &UInventoryOverlayWidget::OnItemSelected);
            
            // Adicionar ao ScrollBox
            ItemsScrollBox->AddChild(EntryWidget);
            
            // Manter referência
            ItemEntries.Add(EntryWidget);
            
            UE_LOG(LogTemp, Log, TEXT("Created item entry for: %s"), 
                Item.ItemData ? *Item.ItemData->ItemName.ToString() : TEXT("Unknown"));
        }
    }
    
            UE_LOG(LogTemp, Log, TEXT("Populated items list with %d items for category %s"), 
        FilteredItems.Num(), *GetCategoryDisplayName(Category));
    
    // Auto-hover no primeiro item (mais robusto)
    if (ItemEntries.Num() > 0)
    {
        bFirstItemAutoHover = true;
        
        // IMPORTANTE: Configurar o primeiro item como ativo
        ActiveHoverItem = ItemEntries[0].Get();
        if (ActiveHoverItem)
        {
            ActiveHoverItem->SetSelectedVisual(true);
            
            // Aplicar seleção do primeiro item
            ApplyItemSelection(ActiveHoverItem->GetCurrentItem());
            
            // Log para debug
            UE_LOG(LogTemp, Log, TEXT("InventoryOverlayWidget: Auto-hover inicial ativado no primeiro item"));
        }
    }
}

void UInventoryOverlayWidget::PopulateItemsListBySubtype(const FString& SubtypeName)
{
    if (!ItemsScrollBox || !ItemEntryClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ItemsScrollBox or ItemEntryClass not found"));
        return;
    }
    
    // Limpar lista existente
    ClearItemsList();
    
    // Obter itens do inventário
    UInventorySubsystem* InventorySystem = GetInventorySubsystem();
    if (!InventorySystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("InventorySubsystem not found"));
        return;
    }
    
    // Obter todos os itens e filtrar por categoria + subtype
    TArray<FInventoryItem> AllItems = InventorySystem->GetAllItems();
    TArray<FInventoryItem> FilteredItems;
    
    for (const FInventoryItem& Item : AllItems)
    {
        if (Item.ItemData && Item.ItemData->ItemCategory == CurrentSelectedCategory)
        {
            // Filtrar por subtype baseado na categoria
            bool bMatchesSubtype = false;
            
            switch (CurrentSelectedCategory)
            {
                case EItemCategory::Weapon:
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->WeaponType) == SubtypeName);
                    break;
                    
                case EItemCategory::Armor:
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->ArmorType) == SubtypeName);
                    break;
                    
                case EItemCategory::Boots:
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->BootsType) == SubtypeName);
                    break;
                    
                case EItemCategory::Accessory:
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->AccessoryType) == SubtypeName);
                    break;
                    
                case EItemCategory::Ring:
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->RingType) == SubtypeName);
                    break;
                    
                case EItemCategory::Consumable:
                    bMatchesSubtype = (UIUtilities::GetEnumDisplayName(Item.ItemData->ConsumableType) == SubtypeName);
                    break;
                    
                default:
                    bMatchesSubtype = false;
                    break;
            }
            
            if (bMatchesSubtype)
            {
                FilteredItems.Add(Item);
            }
        }
    }
    
    // Criar entries para cada item filtrado
    for (const FInventoryItem& Item : FilteredItems)
    {
        UInventoryItemEntryWidget* EntryWidget = CreateWidget<UInventoryItemEntryWidget>(this, ItemEntryClass);
        if (EntryWidget)
        {
            // Configurar o entry com o item
            EntryWidget->Setup(Item);
            
            // Vincular evento de seleção
            EntryWidget->OnItemSelected.AddDynamic(this, &UInventoryOverlayWidget::OnItemSelected);
            
            // Adicionar ao ScrollBox
            ItemsScrollBox->AddChild(EntryWidget);
            
            // Manter referência
            ItemEntries.Add(EntryWidget);
            
            UE_LOG(LogTemp, Log, TEXT("Created subtype item entry for: %s"), 
                Item.ItemData ? *Item.ItemData->ItemName.ToString() : TEXT("Unknown"));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Populated items list with %d items for subtype %s"), 
        FilteredItems.Num(), *SubtypeName);
    
    // Auto-hover no primeiro item (mais robusto)
    if (ItemEntries.Num() > 0)
    {
        bFirstItemAutoHover = true;
        
        // IMPORTANTE: Configurar o primeiro item como ativo
        ActiveHoverItem = ItemEntries[0].Get();
        if (ActiveHoverItem)
        {
            ActiveHoverItem->SetSelectedVisual(true);
            
            // Aplicar seleção do primeiro item
            ApplyItemSelection(ActiveHoverItem->GetCurrentItem());
            
            // Log para debug
            UE_LOG(LogTemp, Log, TEXT("InventoryOverlayWidget: Auto-hover inicial ativado no primeiro item do subtype"));
        }
    }
}

void UInventoryOverlayWidget::ClearItemsList()
{
    if (!ItemsScrollBox)
    {
        return;
    }
    
    // Limpar hover antes de limpar a lista
    ClearItemListHover();
    
    // Limpar referências aos entries
    for (TWeakObjectPtr<UInventoryItemEntryWidget>& EntryPtr : ItemEntries)
    {
        if (UInventoryItemEntryWidget* Entry = EntryPtr.Get())
        {
            Entry->OnItemSelected.RemoveAll(this);
            Entry->RemoveFromParent();
        }
    }
    
    // Limpar arrays
    ItemEntries.Empty();
    ItemsScrollBox->ClearChildren();
    
    UE_LOG(LogTemp, Log, TEXT("Cleared items list"));
}

void UInventoryOverlayWidget::ClearItemListHover()
{
    if (ActiveHoverItem)
    {
        ActiveHoverItem->SetSelectedVisual(false);
        ActiveHoverItem = nullptr;
    }
    
    bFirstItemAutoHover = false;
    
    UE_LOG(LogTemp, Log, TEXT("Cleared item list selection"));
}

