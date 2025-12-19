#include "UI/Menus/GameTabMenuWidget.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "EnhancedInputSubsystems.h"

// RPG Includes
#include "Player/RPGPlayerController.h"
#include "UI/HUD/RPGHUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "HAL/PlatformTime.h"

UGameTabMenuWidget::UGameTabMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UGameTabMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Permitir que o widget receba input de teclado
    SetIsFocusable(true);

    if (!TabContentSwitcher)
    {
    }

    // Inicializar sistema de abas
    InitializeTabs();

    // Estado inicial: mostrar somente header (botões), esconder conteúdo
    if (TabContentSwitcher)
    {
        TabContentSwitcher->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (TabButtonsContainer)
    {
        TabButtonsContainer->SetVisibility(ESlateVisibility::Visible);
    }
    if (BackButton)
    {
        BackButton->SetVisibility(ESlateVisibility::Collapsed);
        BackButton->SetIsEnabled(false);
    }
}

void UGameTabMenuWidget::ShowTabButtons(bool bShow)
{
    if (TabButtonsContainer)
    {
        TabButtonsContainer->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}

void UGameTabMenuWidget::OnBackButtonClicked()
{
    // Retornar ao estado inicial
    if (TabContentSwitcher)
    {
        TabContentSwitcher->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (TabButtonsContainer)
    {
        TabButtonsContainer->SetVisibility(ESlateVisibility::Visible);
    }
    if (BackButton)
    {
        BackButton->SetVisibility(ESlateVisibility::Collapsed);
        BackButton->SetIsEnabled(false);
    }
    ActiveTabButton.Reset();
    UpdateTabButtonStyles();
}

// === SISTEMA INTERNO ===

void UGameTabMenuWidget::InitializeTabs()
{
    // Limpar mapas
    ButtonToClassMap.Empty();
    ButtonToInstanceMap.Empty();
    LastAccessTimeMap.Empty();
    ActiveTabButton.Reset();
    TabContentSwitcher->ClearChildren();
    
    // Array para manter ordem das abas
    TArray<TPair<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>>> ButtonClassPairs;

    // Adicionar abas disponíveis (apenas mapeamento, sem criar widgets)
    if (StatusButton && StatusContentClass)
    {
        ButtonClassPairs.Add(TPair<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>>(StatusButton, StatusContentClass));
    }
    if (SkillsButton && SkillsContentClass)
    {
        ButtonClassPairs.Add(TPair<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>>(SkillsButton, SkillsContentClass));
    }
    if (SkillTreeButton && SkillTreeContentClass)
    {
        ButtonClassPairs.Add(TPair<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>>(SkillTreeButton, SkillTreeContentClass));
    }
    if (InventoryButton && InventoryContentClass)
    {
        ButtonClassPairs.Add(TPair<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>>(InventoryButton, InventoryContentClass));
    }
    if (RecordsButton && RecordsContentClass)
    {
        ButtonClassPairs.Add(TPair<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>>(RecordsButton, RecordsContentClass));
    }
    if (MapButton && MapContentClass)
    {
        ButtonClassPairs.Add(TPair<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>>(MapButton, MapContentClass));
    }
    if (PartyButton && PartyContentClass)
    {
        ButtonClassPairs.Add(TPair<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>>(PartyButton, PartyContentClass));
    }
    if (SystemButton && SystemContentClass)
    {
        ButtonClassPairs.Add(TPair<TWeakObjectPtr<UButton>, TSubclassOf<UUserWidget>>(SystemButton, SystemContentClass));
    }

    // Apenas mapear botões para classes (sem criar widgets)
    for (int32 i = 0; i < ButtonClassPairs.Num(); i++)
    {
        const auto& Pair = ButtonClassPairs[i];
        TWeakObjectPtr<UButton> Button = Pair.Key;
        TSubclassOf<UUserWidget> ContentClass = Pair.Value;

        if (Button.IsValid() && ContentClass)
        {
            ButtonToClassMap.Add(Button, ContentClass);
        }
    }

    // Fazer bind dos eventos
    if (StatusButton && ButtonToClassMap.Contains(StatusButton))
    {
        StatusButton->OnClicked.RemoveAll(this);
        StatusButton->OnClicked.AddDynamic(this, &UGameTabMenuWidget::OnStatusButtonClicked);
        StatusButton->SetIsEnabled(true);
        StatusButton->SetVisibility(ESlateVisibility::Visible);
    }
    else if (StatusButton)
    {
        StatusButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (SkillsButton && ButtonToClassMap.Contains(SkillsButton))
    {
        SkillsButton->OnClicked.RemoveAll(this);
        SkillsButton->OnClicked.AddDynamic(this, &UGameTabMenuWidget::OnSkillsButtonClicked);
        SkillsButton->SetIsEnabled(true);
        SkillsButton->SetVisibility(ESlateVisibility::Visible);
    }
    else if (SkillsButton)
    {
        SkillsButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (SkillTreeButton && ButtonToClassMap.Contains(SkillTreeButton))
    {
        SkillTreeButton->OnClicked.RemoveAll(this);
        SkillTreeButton->OnClicked.AddDynamic(this, &UGameTabMenuWidget::OnSkillTreeButtonClicked);
        SkillTreeButton->SetIsEnabled(true);
        SkillTreeButton->SetVisibility(ESlateVisibility::Visible);
    }
    else if (SkillTreeButton)
    {
        SkillTreeButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (InventoryButton && ButtonToClassMap.Contains(InventoryButton))
    {
        InventoryButton->OnClicked.RemoveAll(this);
        InventoryButton->OnClicked.AddDynamic(this, &UGameTabMenuWidget::OnInventoryButtonClicked);
        InventoryButton->SetIsEnabled(true);
        InventoryButton->SetVisibility(ESlateVisibility::Visible);
    }
    else if (InventoryButton)
    {
        InventoryButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (RecordsButton && ButtonToClassMap.Contains(RecordsButton))
    {
        RecordsButton->OnClicked.RemoveAll(this);
        RecordsButton->OnClicked.AddDynamic(this, &UGameTabMenuWidget::OnRecordsButtonClicked);
        RecordsButton->SetIsEnabled(true);
        RecordsButton->SetVisibility(ESlateVisibility::Visible);
    }
    else if (RecordsButton)
    {
        RecordsButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (MapButton && ButtonToClassMap.Contains(MapButton))
    {
        MapButton->OnClicked.RemoveAll(this);
        MapButton->OnClicked.AddDynamic(this, &UGameTabMenuWidget::OnMapButtonClicked);
        MapButton->SetIsEnabled(true);
        MapButton->SetVisibility(ESlateVisibility::Visible);
    }
    else if (MapButton)
    {
        MapButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (PartyButton && ButtonToClassMap.Contains(PartyButton))
    {
        PartyButton->OnClicked.RemoveAll(this);
        PartyButton->OnClicked.AddDynamic(this, &UGameTabMenuWidget::OnPartyButtonClicked);
        PartyButton->SetIsEnabled(true);
        PartyButton->SetVisibility(ESlateVisibility::Visible);
    }
    else if (PartyButton)
    {
        PartyButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    if (SystemButton && ButtonToClassMap.Contains(SystemButton))
    {
        SystemButton->OnClicked.RemoveAll(this);
        SystemButton->OnClicked.AddDynamic(this, &UGameTabMenuWidget::OnSystemButtonClicked);
        SystemButton->SetIsEnabled(true);
        SystemButton->SetVisibility(ESlateVisibility::Visible);
    }
    else if (SystemButton)
    {
        SystemButton->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Back button opcional
    if (BackButton)
    {
        BackButton->OnClicked.RemoveAll(this);
        BackButton->OnClicked.AddDynamic(this, &UGameTabMenuWidget::OnBackButtonClicked);
    }
    
    // Se lazy loading está desabilitado, carregar primeira aba automaticamente
    // COMENTADO: Não carregar automaticamente para evitar exibição indesejada
    /*
    if (!bEnableLazyLoading && ButtonToClassMap.Num() > 0)
    {
        for (const auto& Pair : ButtonToClassMap)
        {
            if (Pair.Key.IsValid())
            {
                LoadTabOnDemand(Pair.Key.Get());
                break;
            }
        }
    }
    */
}

void UGameTabMenuWidget::SelectDefaultTab()
{
    // Ordem de preferência
    TArray<UButton*> Preference;
    Preference.Reserve(7);
    Preference.Add(StatusButton.Get());
    Preference.Add(SkillsButton.Get());
    Preference.Add(InventoryButton.Get());
    Preference.Add(RecordsButton.Get());
    Preference.Add(MapButton.Get());
    Preference.Add(PartyButton.Get());
    Preference.Add(SystemButton.Get());

    for (UButton* Button : Preference)
    {
        if (Button && ButtonToClassMap.Contains(Button))
        {
            SetActiveTab(Button);
            return;
        }
    }
    // Fallback: primeira do mapa
    if (ButtonToClassMap.Num() > 0)
    {
        for (const auto& Pair : ButtonToClassMap)
        {
            if (Pair.Key.IsValid())
            {
                SetActiveTab(Pair.Key.Get());
                break;
            }
        }
    }
}

void UGameTabMenuWidget::LoadTabOnDemand(UButton* TargetButton)
{
    if (!TargetButton)
    {
        return;
    }
    
    // Verificar se o widget já está carregado
    TWeakObjectPtr<UButton> WeakButton(TargetButton);
    if (ButtonToInstanceMap.Contains(WeakButton) && ButtonToInstanceMap[WeakButton].IsValid())
    {
        return;
    }
    
    // Verificar se a classe existe
    if (!ButtonToClassMap.Contains(WeakButton))
    {
        return;
    }
    
    TSubclassOf<UUserWidget> WidgetClass = ButtonToClassMap[WeakButton];
    if (!WidgetClass)
    {
        return;
    }
    
    // Gerenciar cache antes de carregar novo widget
    ManageWidgetCache();
    
    // Criar widget
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(this, WidgetClass);
    if (NewWidget)
    {
        // Adicionar ao switcher
        TabContentSwitcher->AddChild(NewWidget);
        
        // Armazenar instância
        ButtonToInstanceMap.Add(WeakButton, NewWidget);
        
        // Atualizar timestamp de acesso
        LastAccessTimeMap.Add(WeakButton, FPlatformTime::Seconds());
    }
}

void UGameTabMenuWidget::ManageWidgetCache()
{
    if (!bEnableLazyLoading)
    {
        return; // Sistema de cache desabilitado
    }
    
    // REMOVER TODOS os widgets exceto o ativo
    TArray<TWeakObjectPtr<UButton>> ButtonsToRemove;
    
    for (const auto& Pair : ButtonToInstanceMap)
    {
        TWeakObjectPtr<UButton> Button = Pair.Key;
        
        // Pular se é o botão ativo
        if (Button == ActiveTabButton)
        {
            continue;
        }
        
        // Marcar para remoção
        ButtonsToRemove.Add(Button);
    }
    
    // Remover widgets marcados
    for (TWeakObjectPtr<UButton> ButtonToRemove : ButtonsToRemove)
    {
        TWeakObjectPtr<UUserWidget> WidgetToRemove = ButtonToInstanceMap[ButtonToRemove];
        if (WidgetToRemove.IsValid())
        {
            // Remover do switcher
            TabContentSwitcher->RemoveChild(WidgetToRemove.Get());
            
            // Remover dos mapas
            ButtonToInstanceMap.Remove(ButtonToRemove);
            LastAccessTimeMap.Remove(ButtonToRemove);
        }
    }
}

// === CALLBACKS DOS BOTÕES ===

void UGameTabMenuWidget::OnStatusButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("🎮 OnStatusButtonClicked"));
    SetActiveTab(StatusButton.Get());
}

void UGameTabMenuWidget::OnSkillsButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("🎮 OnSkillsButtonClicked"));
    SetActiveTab(SkillsButton.Get());
}

void UGameTabMenuWidget::OnSkillTreeButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("🎮 OnSkillTreeButtonClicked"));
    SetActiveTab(SkillTreeButton.Get());
}

void UGameTabMenuWidget::OnInventoryButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("🎮 OnInventoryButtonClicked"));
    SetActiveTab(InventoryButton.Get());
}

void UGameTabMenuWidget::OnRecordsButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("🎮 OnRecordsButtonClicked"));
    SetActiveTab(RecordsButton.Get());
}

void UGameTabMenuWidget::OnMapButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("🎮 OnMapButtonClicked"));
    SetActiveTab(MapButton.Get());
}

void UGameTabMenuWidget::OnPartyButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("🎮 OnPartyButtonClicked"));
    SetActiveTab(PartyButton.Get());
}

void UGameTabMenuWidget::OnSystemButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("🎮 OnSystemButtonClicked"));
    SetActiveTab(SystemButton.Get());
}

void UGameTabMenuWidget::SetActiveTab(UButton* TargetButton)
{
    if (!TargetButton || !TabContentSwitcher)
    {
        return;
    }

    TWeakObjectPtr<UButton> WeakButton(TargetButton);
    
    // Carregar widget sob demanda se necessário
    if (!ButtonToInstanceMap.Contains(WeakButton))
    {
        LoadTabOnDemand(TargetButton);
    }
    
    TWeakObjectPtr<UUserWidget>* WidgetPtr = ButtonToInstanceMap.Find(WeakButton);
    
    if (!WidgetPtr || !WidgetPtr->IsValid())
    {
        return;
    }

    UUserWidget* TargetWidget = WidgetPtr->Get();
    
    // Atualizar timestamp de acesso
    LastAccessTimeMap.Add(WeakButton, FPlatformTime::Seconds());
    
    // Definir widget ativo
    TabContentSwitcher->SetActiveWidget(TargetWidget);
    
    // Forçar visibilidade
    TabContentSwitcher->SetVisibility(ESlateVisibility::Visible);
    TargetWidget->SetVisibility(ESlateVisibility::Visible);
    
    // Atualizar botão ativo
    ActiveTabButton = WeakButton;
    UpdateTabButtonStyles();
    
    // Gerenciar cache APÓS definir o ativo
    ManageWidgetCache();

    // Esconder container de botões quando uma aba é ativada
    if (TabButtonsContainer)
    {
        TabButtonsContainer->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (BackButton)
    {
        BackButton->SetVisibility(ESlateVisibility::Visible);
        BackButton->SetIsEnabled(true);
    }
}

void UGameTabMenuWidget::UpdateTabButtonStyles()
{
    for (const auto& Pair : ButtonToClassMap)
    {
        TWeakObjectPtr<UButton> Button = Pair.Key;
        if (Button.IsValid())
        {
            bool bIsActive = (Button == ActiveTabButton);
            
            if (bIsActive)
            {
                Button->SetBackgroundColor(FLinearColor::White);
            }
            else
            {
                // Estado inicial: sem aba ativa, manter estilo neutro
                Button->SetBackgroundColor(bIsActive ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
            }
        }
    }
}

void UGameTabMenuWidget::CloseMenu()
{
    // Remover da viewport
    RemoveFromParent();

    // Notificar PlayerController
    if (ARPGPlayerController* PC = Cast<ARPGPlayerController>(GetOwningPlayer()))
    {
        // Voltar ao modo de jogo
        FInputModeGameOnly InputModeData;
        PC->SetInputMode(InputModeData);
        PC->bShowMouseCursor = false;

        // Mostrar HUD novamente
        if (ARPGHUD* RPGHud = Cast<ARPGHUD>(PC->GetHUD()))
        {
            RPGHud->SetGameplayHUDVisibility(true);
        }
    }
}

FReply UGameTabMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Tab)
    {
        RemoveFromParent(); // Fecha o menu
        if (APlayerController* PC = GetOwningPlayer())
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->bShowMouseCursor = false;
        }
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

 