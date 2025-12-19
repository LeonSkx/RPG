#include "UI/Characters/BaseOverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameplayTagsManager.h"
#include "Components/Border.h"
#include "Components/Overlay.h"


#include "AbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Party/PartySubsystem.h"
#include "Progression/ProgressionSubsystem.h"
#include "UI/Characters/Party/PartyMemberStatusWidget.h"

#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "Inventory/Core/InventorySubsystem.h"
#include "Components/SkillEquipmentComponent.h"
#include "UI/Widget/SkillIconWidget.h"
// Define maximum party size if not available from subsystem easily
const int32 MAX_PARTY_SIZE_FOR_UI = 4; 

// Helper to get object name safely
FString GetObjName(const UObject* Obj)
{
    return Obj ? Obj->GetName() : FString(TEXT("nullptr"));
}

void UBaseOverlayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>();
    }
    
    if (!PartySubsystem)
    {
        return;
    }

    // --- Pré-alocar Widgets da Party ---
    if (PartyMemberStatusClass && PartyMembersContainer)
    {
        int32 NumToPreallocate = MAX_PARTY_SIZE_FOR_UI - 1; // Pool for inactive members
        PartyMemberWidgetPool.Reserve(NumToPreallocate); // Reserve space in array

        for (int32 i = 0; i < NumToPreallocate; ++i)
        {
            UPartyMemberStatusWidget* PoolWidget = CreateWidget<UPartyMemberStatusWidget>(this, PartyMemberStatusClass);
            if (PoolWidget)
            {
                PoolWidget->SetVisibility(ESlateVisibility::Collapsed); // Start hidden
                PartyMemberWidgetPool.Add(PoolWidget);
                // Do not add to container yet
            }
        }
    }
    // ----------------------------------
    
    // Subscribe to active party member changes (prevent duplicate bindings)
    if (PartySubsystem)
    {
        PartySubsystem->OnActivePartyMemberChanged.RemoveDynamic(this, &UBaseOverlayWidget::SetOwnerCharacter);
        PartySubsystem->OnActivePartyMemberChanged.AddDynamic(this, &UBaseOverlayWidget::SetOwnerCharacter);
        // Set initial owner to currently active member
        SetOwnerCharacter(PartySubsystem->GetActivePartyMember());
    }
    
    // Conectar aos delegates (incluindo Gold)
    BindDelegates();
}

void UBaseOverlayWidget::NativeDestruct()
{
    UnbindDelegates();
    
    // Reset widgets in pool before destroying the overlay itself
    HideAndResetPartyMemberWidgets(); 
    // Clear the TArray itself
    PartyMemberWidgetPool.Empty(); 
    
    ActiveCharacter = nullptr;
    PartySubsystem = nullptr;
    
    Super::NativeDestruct();
}

void UBaseOverlayWidget::SetOwnerCharacter(ARPGCharacter* NewOwner)
{
    // Desconectar delegados do personagem anterior
    if (ActiveCharacter)
    {
        // Remover delegado de inicialização
        ActiveCharacter->OnAttributesInitialized.RemoveDynamic(this, &UBaseOverlayWidget::OnCharacterAttributesReady);
        
        // Remover delegados de atributos se houver ASC
        if (UAbilitySystemComponent* OldASC = ActiveCharacter->GetAbilitySystemComponent())
        {
            OldASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetHealthAttribute()).RemoveAll(this);
            OldASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
            OldASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetManaAttribute()).RemoveAll(this);
            OldASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetMaxManaAttribute()).RemoveAll(this);
            	// Energy removido - Mana já serve para recursos
        }
    }
    
    // Atualizar referência
    ActiveCharacter = NewOwner;
    
    // Conectar ao novo personagem
    if (ActiveCharacter)
    {
        // Conectar ao delegado de inicialização
        ActiveCharacter->OnAttributesInitialized.AddDynamic(this, &UBaseOverlayWidget::OnCharacterAttributesReady);
        
        // Verificar se os atributos já estão inicializados
        UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent();
        bool bAttributesReady = ASC && ASC->GetAttributeSet(URPGAttributeSet::StaticClass());
        
        if (bAttributesReady)
        {
            // Inicializar UI imediatamente
            OnCharacterAttributesReady();
        }
        else
        {
            // Ocultando widgets de party enquanto aguarda inicialização
            HideAndResetPartyMemberWidgets();
        }
    }
    else
    {
        HideAndResetPartyMemberWidgets(); 
    }
}

void UBaseOverlayWidget::HandleHealthChanged(float Current, float Max)
{
    UpdateAttributeValue(HealthProgressBar, HealthText, Current, Max);
}

void UBaseOverlayWidget::HandleManaChanged(float Current, float Max)
{
    UpdateAttributeValue(ManaProgressBar, ManaText, Current, Max);
}

// Energy removido - Mana já serve para recursos

void UBaseOverlayWidget::HandleXPChanged(ARPGCharacter* Character, int32 CurrentXP, int32 XPForNextLevel)
{
    // Só atualizar se for o personagem ativo
    if (Character != ActiveCharacter)
    {
        return;
    }



    if (XPProgressBar)
    {
        float Progress = 0.0f;
        if (XPForNextLevel > 0)
        {
            // Calcular XP relativo ao nível atual
            int32 XPForCurrentLevel = 0;
            if (UGameInstance* GameInstance = GetGameInstance())
            {
                if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
                {
                    int32 CurrentLevel = ProgressionSubsystem->GetCharacterLevel(Character);
                    XPForCurrentLevel = ProgressionSubsystem->CalculateXPForLevel(CurrentLevel);
                }
            }
            
            int32 XPInCurrentLevel = CurrentXP - XPForCurrentLevel;
            int32 XPNeededForNextLevel = XPForNextLevel - XPForCurrentLevel;
            
            if (XPNeededForNextLevel > 0)
            {
                Progress = static_cast<float>(XPInCurrentLevel) / static_cast<float>(XPNeededForNextLevel);
            }
            
            Progress = FMath::Clamp(Progress, 0.0f, 1.0f);
        }
        
        XPProgressBar->SetPercent(Progress);
    }

    if (XPText)
    {
        // Calcular XP no nível atual vs XP necessário para o próximo nível
        int32 XPForCurrentLevel = 0;
        int32 XPNeededForNextLevel = 0;
        
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
            {
                int32 CurrentLevel = ProgressionSubsystem->GetCharacterLevel(Character);
                XPForCurrentLevel = ProgressionSubsystem->CalculateXPForLevel(CurrentLevel);
                XPNeededForNextLevel = XPForNextLevel - XPForCurrentLevel;
            }
        }
        
        int32 XPInCurrentLevel = CurrentXP - XPForCurrentLevel;
        
        XPText->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), XPInCurrentLevel, XPNeededForNextLevel)));
    }
}

void UBaseOverlayWidget::HandleLevelChanged(ARPGCharacter* Character, int32 NewLevel)
{
    // Só atualizar se for o personagem ativo
    if (Character != ActiveCharacter)
    {
        return;
    }



    if (LevelText)
    {
        LevelText->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewLevel)));
    }
}

void UBaseOverlayWidget::HandleGoldChanged(int32 NewAmount)
{
    UE_LOG(LogTemp, Log, TEXT("BaseOverlayWidget: HandleGoldChanged chamado - Novo valor: %d"), NewAmount);
    
    if (GoldText)
    {
        GoldText->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewAmount)));
        UE_LOG(LogTemp, Log, TEXT("BaseOverlayWidget: GoldText atualizado com sucesso"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BaseOverlayWidget: GoldText é nullptr!"));
    }
}

void UBaseOverlayWidget::BindDelegates()
{
    // Sistema MVC: binding é feito via Widget Controller
    
    // Conectar ao SharedInventorySubsystem para Gold
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UInventorySubsystem* SharedInventory = GameInstance->GetSubsystem<UInventorySubsystem>())
        {
            SharedInventory->OnGoldChanged.AddDynamic(this, &UBaseOverlayWidget::HandleGoldChanged);
            
            // Atualizar valor inicial
            int32 CurrentGold = SharedInventory->GetGold();
            HandleGoldChanged(CurrentGold);
            
            UE_LOG(LogTemp, Log, TEXT("BaseOverlayWidget: Conectado ao SharedInventory - Gold atual: %d"), CurrentGold);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("BaseOverlayWidget: SharedInventorySubsystem não encontrado!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BaseOverlayWidget: GameInstance não encontrado!"));
    }
}

void UBaseOverlayWidget::UnbindDelegates()
{
    // Remove binding for active party member change
    if (PartySubsystem)
    {
        PartySubsystem->OnActivePartyMemberChanged.RemoveDynamic(this, &UBaseOverlayWidget::SetOwnerCharacter);
    }
    // Remove attribute initialization binding
    if (ActiveCharacter)
    {
        ActiveCharacter->OnAttributesInitialized.RemoveDynamic(this, &UBaseOverlayWidget::OnCharacterAttributesReady);
    }
    
    // Remove ProgressionSubsystem bindings
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
        {
            ProgressionSubsystem->OnCharacterXPChanged.RemoveDynamic(this, &UBaseOverlayWidget::HandleXPChanged);
            ProgressionSubsystem->OnCharacterLevelUp.RemoveDynamic(this, &UBaseOverlayWidget::HandleLevelChanged);
        }
        
        // Remove SharedInventorySubsystem bindings
        if (UInventorySubsystem* SharedInventory = GameInstance->GetSubsystem<UInventorySubsystem>())
        {
            SharedInventory->OnGoldChanged.RemoveDynamic(this, &UBaseOverlayWidget::HandleGoldChanged);
        }
    }
    
}

void UBaseOverlayWidget::SetupInitialValues()
{
    // Sistema MVC: valores iniciais vêm do Widget Controller via BroadcastInitialValues()
}

void UBaseOverlayWidget::UpdateAttributeText(UTextBlock* TextWidget, float Current, float Max)
{
    if (TextWidget)
    {
        TextWidget->SetText(FText::FromString(FString::Printf(TEXT("%.0f/%.0f"), Current, Max)));
    }
}

void UBaseOverlayWidget::UpdateActiveCharacterBars()
{
    // Sistema MVC: atualização via Widget Controller delegates
}

void UBaseOverlayWidget::HideAndResetPartyMemberWidgets()
{
    // Clear the visual container first
    if (PartyMembersContainer)
    {
        PartyMembersContainer->ClearChildren();
    }

    // Iterate through the pool, reset and hide each widget
    for (UPartyMemberStatusWidget* Widget : PartyMemberWidgetPool)
    {
        if (Widget)
        {
            Widget->UnbindDelegates(); // Call the existing UnbindDelegates function
            Widget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

void UBaseOverlayWidget::UpdatePartyMemberDisplay()
{
    if (!PartyMembersContainer || !PartySubsystem || !PartyMemberStatusClass)
    {
        return;
    }
    
    // Clear the visual container before repopulating
    PartyMembersContainer->ClearChildren();

    const TArray<ARPGCharacter*>& Members = PartySubsystem->GetPartyMembers();
    TSet<UPartyMemberStatusWidget*> UsedWidgets; // Keep track of widgets used this update
    int32 PoolIndex = 0;

    for (ARPGCharacter* Member : Members)
    {
        // Only display widgets for INACTIVE members
        if (Member && Member != ActiveCharacter)
        {
            if (PoolIndex < PartyMemberWidgetPool.Num())
            {
                UPartyMemberStatusWidget* WidgetToUse = PartyMemberWidgetPool[PoolIndex];
                if (WidgetToUse)
                {
                    // Setup the widget with the current member data (this also binds delegates)
                    WidgetToUse->SetupMember(Member); 
                    // Make it visible
                    WidgetToUse->SetVisibility(ESlateVisibility::Visible);
                    // Add it to the container
                    PartyMembersContainer->AddChild(WidgetToUse);
                    // Mark as used
                    UsedWidgets.Add(WidgetToUse);
                    PoolIndex++;
                }
                else
                {
                     PoolIndex++; // Skip this null entry
                }
            }
        }
    }
    
    // Hide any remaining unused widgets in the pool
    for(UPartyMemberStatusWidget* PoolWidget : PartyMemberWidgetPool)
    {
        if(PoolWidget && !UsedWidgets.Contains(PoolWidget))
        {
            PoolWidget->SetVisibility(ESlateVisibility::Collapsed);
            // Optionally call UnbindDelegates here too, although SetupMember should handle it
            PoolWidget->UnbindDelegates(); 
        }
    }
}

void UBaseOverlayWidget::OnCharacterAttributesReady()
{
    if (!ActiveCharacter)
    {
        return;
    }
    
    // Verificar se temos ASC
    UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }
    
    // Conectar diretamente ao ASC, independentemente do Widget Controller
    // (o método BindToController já verifica duplicações)
    BindToController();
    
    // Atualizar valores iniciais é chamado dentro de BindToController
    
    // Update the display for inactive party members (uses the pool)
    UpdatePartyMemberDisplay();
    
    // Conectar ao ProgressionSubsystem para atualizações de XP
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
        {
            // Remover conexões anteriores para evitar duplicatas
            ProgressionSubsystem->OnCharacterXPChanged.RemoveDynamic(this, &UBaseOverlayWidget::HandleXPChanged);
            ProgressionSubsystem->OnCharacterLevelUp.RemoveDynamic(this, &UBaseOverlayWidget::HandleLevelChanged);
            
            // Conectar aos eventos de XP e Level
            ProgressionSubsystem->OnCharacterXPChanged.AddDynamic(this, &UBaseOverlayWidget::HandleXPChanged);
            ProgressionSubsystem->OnCharacterLevelUp.AddDynamic(this, &UBaseOverlayWidget::HandleLevelChanged);
            
            // Atualizar valores iniciais de XP
            int32 CurrentXP = ProgressionSubsystem->GetCharacterXP(ActiveCharacter);
            int32 XPForNextLevel = ProgressionSubsystem->CalculateXPForNextLevel(ActiveCharacter);
            int32 CurrentLevel = ProgressionSubsystem->GetCharacterLevel(ActiveCharacter);
            
            // Calcular XP no nível atual vs XP necessário para o próximo nível
            int32 XPForCurrentLevel = ProgressionSubsystem->CalculateXPForLevel(CurrentLevel);
            int32 XPNeededForNextLevel = XPForNextLevel - XPForCurrentLevel;
            int32 XPInCurrentLevel = CurrentXP - XPForCurrentLevel;
            
            HandleXPChanged(ActiveCharacter, CurrentXP, XPForNextLevel);
            HandleLevelChanged(ActiveCharacter, CurrentLevel);
        }
    }
    
    // Forçar atualização após um breve atraso (garantir que os valores sejam atualizados mesmo se houver latência)
    GetWorld()->GetTimerManager().SetTimer(
        UpdateTimerHandle, 
        this, 
        &UBaseOverlayWidget::ForceUpdateAllValues, 
        0.5f, 
        false);
}

void UBaseOverlayWidget::UpdateAttributeValue(UProgressBar* Bar, UTextBlock* Text, float Current, float Max)
{
    if (Bar)
    {
        float Percent = Max > 0.0f ? (Current / Max) : 0.0f;
        Bar->SetPercent(Percent);
    }
    
    if (Text)
    {
        Text->SetText(FText::FromString(FString::Printf(TEXT("%.0f/%.0f"), Current, Max)));
    }
}

void UBaseOverlayWidget::UpdateCharacterName(const FString& NewName)
{
    if (CharacterNameText)
    {
        CharacterNameText->SetText(FText::FromString(NewName));
    }
}

void UBaseOverlayWidget::ForceUpdateAllValues()
{
    if (ActiveCharacter)
    {
        if (UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent())
        {
            float Current = ASC->GetNumericAttribute(URPGAttributeSet::GetHealthAttribute());
            float Max = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxHealthAttribute());
            HandleHealthChanged(Current, Max);
            
            Current = ASC->GetNumericAttribute(URPGAttributeSet::GetManaAttribute());
            Max = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxManaAttribute());
            HandleManaChanged(Current, Max);
            
            	// Energy removido - Mana já serve para recursos
        }
        
        // Forçar atualização dos valores de XP também
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
            {
                int32 CurrentXP = ProgressionSubsystem->GetCharacterXP(ActiveCharacter);
                int32 XPForNextLevel = ProgressionSubsystem->CalculateXPForNextLevel(ActiveCharacter);
                int32 CurrentLevel = ProgressionSubsystem->GetCharacterLevel(ActiveCharacter);
                

                
                HandleXPChanged(ActiveCharacter, CurrentXP, XPForNextLevel);
                HandleLevelChanged(ActiveCharacter, CurrentLevel);
            }
        }
    }
}

void UBaseOverlayWidget::SetWidgetController(UObject* InWidgetController)
{
    WidgetController = InWidgetController;
    
    // Chamar BindToController explicitamente para garantir que os delegados sejam conectados
    BindToController();
    
    // Notificar o Blueprint
    WidgetControllerSet();
}

void UBaseOverlayWidget::BindToController()
{
    // Sistema dedicado: cada overlay se conecta ao SEU personagem específico
    if (!ActiveCharacter)
    {
        return;
    }

    UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent();
    if (!ASC)
    {
        return;
    }

    // Primeiro, desconectar quaisquer delegados existentes para evitar duplicação
    ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetHealthAttribute())
        .RemoveAll(this);
    ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetMaxHealthAttribute())
        .RemoveAll(this);
    ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetManaAttribute())
        .RemoveAll(this);
    ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetMaxManaAttribute())
        .RemoveAll(this);
    // Energy removido - Mana já serve para recursos


    
    // Conectar diretamente aos atributos do personagem específico
    ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetHealthAttribute())
        .AddUObject(this, &UBaseOverlayWidget::OnHealthChangedDirect);
    
    ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetMaxHealthAttribute())
        .AddUObject(this, &UBaseOverlayWidget::OnMaxHealthChangedDirect);
        
    ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetManaAttribute())
        .AddUObject(this, &UBaseOverlayWidget::OnManaChangedDirect);
        
    ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetMaxManaAttribute())
        .AddUObject(this, &UBaseOverlayWidget::OnMaxManaChangedDirect);
        
    	// Energy removido - Mana já serve para recursos


    
    // Atualizar valores iniciais imediatamente
    UpdateInitialValues();
}

// Implementações das funções de callback do controller
void UBaseOverlayWidget::OnHealthChangedFromController(float NewValue)
{
    // Armazenar valor atual para usar com o máximo quando ele chegar
    CurrentHealthValue = NewValue;
    if (MaxHealthValue > 0.0f)
    {
        HandleHealthChanged(CurrentHealthValue, MaxHealthValue);
    }
}

void UBaseOverlayWidget::OnMaxHealthChangedFromController(float NewValue)
{
    // Armazenar valor máximo e atualizar com o valor atual
    MaxHealthValue = NewValue;
    HandleHealthChanged(CurrentHealthValue, MaxHealthValue);
}

void UBaseOverlayWidget::OnManaChangedFromController(float NewValue)
{
    // Armazenar valor atual para usar com o máximo quando ele chegar
    CurrentManaValue = NewValue;
    if (MaxManaValue > 0.0f)
    {
        HandleManaChanged(CurrentManaValue, MaxManaValue);
    }
}

void UBaseOverlayWidget::OnMaxManaChangedFromController(float NewValue)
{
    // Armazenar valor máximo e atualizar com o valor atual
    MaxManaValue = NewValue;
    HandleManaChanged(CurrentManaValue, MaxManaValue);
}

// Energy removido - Mana já serve para recursos

// Novas funções de callback diretas para personagens específicos
void UBaseOverlayWidget::OnHealthChangedDirect(const FOnAttributeChangeData& Data)
{
    if (!ActiveCharacter) return;
    
    UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent();
    if (!ASC) return;
    
    float CurrentHealth = Data.NewValue;
    float MaxHealth = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxHealthAttribute());
    
    HandleHealthChanged(CurrentHealth, MaxHealth);
}

void UBaseOverlayWidget::OnMaxHealthChangedDirect(const FOnAttributeChangeData& Data)
{
    if (!ActiveCharacter) return;
    
    UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent();
    if (!ASC) return;
    
    float CurrentHealth = ASC->GetNumericAttribute(URPGAttributeSet::GetHealthAttribute());
    float MaxHealth = Data.NewValue;
    
    HandleHealthChanged(CurrentHealth, MaxHealth);
}

void UBaseOverlayWidget::OnManaChangedDirect(const FOnAttributeChangeData& Data)
{
    if (!ActiveCharacter) return;
    
    UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent();
    if (!ASC) return;
    
    float CurrentMana = Data.NewValue;
    float MaxMana = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxManaAttribute());
    
    HandleManaChanged(CurrentMana, MaxMana);
}

void UBaseOverlayWidget::OnMaxManaChangedDirect(const FOnAttributeChangeData& Data)
{
    if (!ActiveCharacter) return;
    
    UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent();
    if (!ASC) return;
    
    float CurrentMana = ASC->GetNumericAttribute(URPGAttributeSet::GetManaAttribute());
    float MaxMana = Data.NewValue;
    
    HandleManaChanged(CurrentMana, MaxMana);
}

// Energy removido - Mana já serve para recursos

void UBaseOverlayWidget::UpdateInitialValues()
{
    if (!ActiveCharacter) 
    {
        return;
    }
    
    UAbilitySystemComponent* ASC = ActiveCharacter->GetAbilitySystemComponent();
    if (!ASC) 
    {
        return;
    }
    
    // Verificar se o AttributeSet está inicializado
    if (!ASC->GetAttributeSet(URPGAttributeSet::StaticClass()))
    {
        return;
    }

    // Atualizar o nome do personagem
    UpdateCharacterName(ActiveCharacter->GetCharacterName());
    
    // Conectar aos eventos do SkillEquipmentComponent
    ConnectToSkillEquipmentEvents();
    
    // Atualizar ícones de habilidades
    UpdateAllSkillIcons();
    
    
    // Atualizar todos os valores iniciais com segurança
    float CurrentHealth = ASC->GetNumericAttribute(URPGAttributeSet::GetHealthAttribute());
    float MaxHealth = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxHealthAttribute());
    if (MaxHealth <= 0)
    {
        MaxHealth = 1.0f; // Evitar divisão por zero
    }
    HandleHealthChanged(CurrentHealth, MaxHealth);
    
    float CurrentMana = ASC->GetNumericAttribute(URPGAttributeSet::GetManaAttribute());
    float MaxMana = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxManaAttribute());
    if (MaxMana <= 0)
    {
        MaxMana = 1.0f;
    }
    HandleManaChanged(CurrentMana, MaxMana);
    
    // Energy removido - Mana já serve para recursos
}

// === SKILL ICONS ===

void UBaseOverlayWidget::UpdateAllSkillIcons()
{
    if (!ActiveCharacter)
    {
        return;
    }
    
    // Obter SkillEquipmentComponent
    USkillEquipmentComponent* SkillComponent = ActiveCharacter->GetSkillEquipmentComponent();
    if (!SkillComponent)
    {
        return;
    }
    
    // Atualizar ícones de todos os widgets gerenciados
    for (USkillIconWidget* IconWidget : ManagedSkillIconWidgets)
    {
        if (IconWidget)
        {
            FName SlotID = IconWidget->GetSlotID();
            UTexture2D* Icon = SkillComponent->GetSlotIcon(SlotID);
            
            IconWidget->SetSkillIcon(Icon);
        }
    }
}

void UBaseOverlayWidget::ConnectToSkillEquipmentEvents()
{
    if (!ActiveCharacter)
    {
        return;
    }
    
    USkillEquipmentComponent* SkillComponent = ActiveCharacter->GetSkillEquipmentComponent();
    if (!SkillComponent)
    {
        return;
    }
    
    // Desconectar primeiro para evitar múltiplas conexões
    DisconnectFromSkillEquipmentEvents();
    
    // Conectar aos delegates
    SkillComponent->OnSkillEquipmentEquipped.AddDynamic(this, &UBaseOverlayWidget::OnSkillEquipped);
    SkillComponent->OnSkillEquipmentUnequipped.AddDynamic(this, &UBaseOverlayWidget::OnSkillUnequipped);
    SkillComponent->OnSkillEquippedInSlot.AddDynamic(this, &UBaseOverlayWidget::OnSkillEquippedInSlot);
}

void UBaseOverlayWidget::BeginDestroy()
{
    // Desconectar todos os delegates antes de destruir
    DisconnectFromSkillEquipmentEvents();
    
    Super::BeginDestroy();
}

void UBaseOverlayWidget::DisconnectFromSkillEquipmentEvents()
{
    if (!ActiveCharacter)
    {
        return;
    }
    
    USkillEquipmentComponent* SkillComponent = ActiveCharacter->GetSkillEquipmentComponent();
    if (!SkillComponent)
    {
        return;
    }
    
    // Desconectar dos delegates
    SkillComponent->OnSkillEquipmentEquipped.RemoveDynamic(this, &UBaseOverlayWidget::OnSkillEquipped);
    SkillComponent->OnSkillEquipmentUnequipped.RemoveDynamic(this, &UBaseOverlayWidget::OnSkillUnequipped);
    SkillComponent->OnSkillEquippedInSlot.RemoveDynamic(this, &UBaseOverlayWidget::OnSkillEquippedInSlot);
}

void UBaseOverlayWidget::OnSkillEquipped(FName CharacterID, FName SlotID, FName SkillID)
{
    // Atualizar ícone do slot específico
    for (USkillIconWidget* IconWidget : ManagedSkillIconWidgets)
    {
        if (IconWidget && IconWidget->GetSlotID() == SlotID)
        {
            if (ActiveCharacter)
            {
                USkillEquipmentComponent* SkillComponent = ActiveCharacter->GetSkillEquipmentComponent();
                if (SkillComponent)
                {
                    UTexture2D* Icon = SkillComponent->GetSlotIcon(SlotID);
                    IconWidget->SetSkillIcon(Icon);
                }
            }
            break;
        }
    }
}

void UBaseOverlayWidget::OnSkillUnequipped(FName CharacterID, FName SlotID, FName SkillID)
{
    // Limpar ícone do slot específico
    for (USkillIconWidget* IconWidget : ManagedSkillIconWidgets)
    {
        if (IconWidget && IconWidget->GetSlotID() == SlotID)
        {
            IconWidget->SetSkillIcon(nullptr);
            break;
        }
    }
}

void UBaseOverlayWidget::OnSkillEquippedInSlot(FName SlotID, FName SkillID, UTexture2D* SkillIcon)
{
    // Atualizar ícone do slot específico
    for (USkillIconWidget* IconWidget : ManagedSkillIconWidgets)
    {
        if (IconWidget && IconWidget->GetSlotID() == SlotID)
        {
            IconWidget->SetSkillIcon(SkillIcon);
            break;
        }
    }
}

void UBaseOverlayWidget::ConnectSkillIconWidgetsArray(const TArray<USkillIconWidget*>& IconWidgetsArray)
{
    // Limpar array atual
    ManagedSkillIconWidgets.Empty();
    
    // Adicionar todos os widgets do array
    for (USkillIconWidget* IconWidget : IconWidgetsArray)
    {
        if (IconWidget)
        {
            ManagedSkillIconWidgets.Add(IconWidget);
        }
    }
}

void UBaseOverlayWidget::AddSkillIconWidget(USkillIconWidget* IconWidget)
{
    if (IconWidget && !ManagedSkillIconWidgets.Contains(IconWidget))
    {
        ManagedSkillIconWidgets.Add(IconWidget);
    }
}

void UBaseOverlayWidget::RemoveSkillIconWidget(USkillIconWidget* IconWidget)
{
    if (IconWidget)
    {
        ManagedSkillIconWidgets.Remove(IconWidget);
    }
}

