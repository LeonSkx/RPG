// Copyright Druid Mechanics

#include "UI/Widget/SkillWidget.h"
#include "UI/WidgetController/SkillTreeWidgetController.h"
#include "UI/Widget/SlotSkillWidget.h"
#include "UI/Widget/SkillDropdownWidget.h"
#include "Character/RPGCharacter.h"
#include "Components/SkillEquipmentComponent.h"
#include "Party/PartySubsystem.h"
#include "Components/ComboBoxString.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"

USkillWidget::USkillWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void USkillWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Inicializar controller automaticamente
    InitializeController();
    
    // Inicializar o ComboBox com os personagens
    InitializeCharacterSelector();
    
    bIsInitialized = true;
}

void USkillWidget::NativeDestruct()
{
    // Desconectar todos os slots
    DisconnectAllSlots();
    
    // Limpar referências
    SkillTreeController = nullptr;
    ManagedSlots.Empty();
    
    bIsInitialized = false;
    
    Super::NativeDestruct();
}

// === CONFIGURAÇÃO ===

void USkillWidget::InitializeWithController(USkillTreeWidgetController* InController)
{
    SetController(InController);
    
    if (IsControllerValid())
    {
        UpdateWidgetFromController();
    }
}

void USkillWidget::SetController(USkillTreeWidgetController* InController)
{
    SkillTreeController = InController;
}

void USkillWidget::InitializeController()
{
    // Se já tem controller, não precisa inicializar
    if (SkillTreeController)
    {
        return;
    }
    
    // Usar a mesma lógica do SkillTreeContentWidget
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            // Criar o controller
            SkillTreeController = NewObject<USkillTreeWidgetController>(this);
            
            if (SkillTreeController)
            {
                // Obter personagem ativo do PartySubsystem (mesmo do SkillTreeContentWidget)
                if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
                {
                    ARPGCharacter* ActiveCharacter = PartySubsystem->GetActiveCharacter();
                    
                    if (ActiveCharacter)
                    {
                        // Inicializar controller com o personagem (mesmo do SkillTreeContentWidget)
                        SkillTreeController->InitializeWithCharacter(ActiveCharacter);
                    }
                }
            }
        }
    }
}

// === GERENCIAMENTO DE SLOTS ===

void USkillWidget::AddSlot(USlotSkillWidget* SlotWidget)
{
    if (!SlotWidget)
    {
        return;
    }
    
    // Verificar se já existe
    if (ManagedSlots.Contains(SlotWidget))
    {
        return;
    }
    
    // Adicionar à lista
    ManagedSlots.Add(SlotWidget);
    
    // Conectar automaticamente
    ConnectSlot(SlotWidget);
}

void USkillWidget::RemoveSlot(USlotSkillWidget* SlotWidget)
{
    if (!SlotWidget)
    {
        return;
    }
    
    // Desconectar primeiro
    DisconnectSlot(SlotWidget);
    
    // Remover da lista
    ManagedSlots.Remove(SlotWidget);
}

void USkillWidget::ConnectAllSlots()
{
    for (USlotSkillWidget* SlotWidget : ManagedSlots)
    {
        if (SlotWidget)
        {
            ConnectSlot(SlotWidget);
        }
    }
    
    // ✅ NOVO: Popular cache após conectar todos os slots
    PopulateInputTagCache();
}

void USkillWidget::DisconnectAllSlots()
{
    for (USlotSkillWidget* SlotWidget : ManagedSlots)
    {
        if (SlotWidget)
        {
            DisconnectSlot(SlotWidget);
        }
    }
}

void USkillWidget::ConnectSlotsArray(const TArray<USlotSkillWidget*>& SlotsArray)
{
    // Limpar slots existentes
    DisconnectAllSlots();
    ManagedSlots.Empty();
    
    // Adicionar novos slots
    for (USlotSkillWidget* SlotWidget : SlotsArray)
    {
        if (SlotWidget)
        {
            AddSlot(SlotWidget);
        }
    }
}

// === EVENTOS DE SLOT ===

void USkillWidget::OnSlotClicked(const FName& SlotID)
{
    // Disparar delegate
    OnSkillWidgetSlotClicked.Broadcast(SlotID);
    
    if (!IsControllerValid())
    {
        return;
    }
    
    // Aqui você pode implementar a lógica para abrir o dropdown
    // Por exemplo: mostrar lista de habilidades desbloqueadas
    TArray<FName> UnlockedSkills = SkillTreeController->GetUnlockedSkills();
    
    // TODO: Implementar abertura do dropdown
    // TODO: Mostrar lista de habilidades para seleção
}

void USkillWidget::OnSkillEquipped(const FName& SlotID, const FName& SkillID)
{
    // Disparar delegate
    OnSkillWidgetSkillEquipped.Broadcast(SlotID, SkillID);
    
    // ✅ NOVO: Atualizar ícones de todos os slots
    UpdateAllSlotIcons();
    
    if (!IsControllerValid())
    {
        return;
    }
    
    // Aqui você pode implementar feedback visual
    // Por exemplo: animação, som, atualização de UI
}

void USkillWidget::OnSkillUnequipped(const FName& SlotID, const FName& SkillID)
{
    // Disparar delegate
    OnSkillWidgetSkillUnequipped.Broadcast(SlotID, SkillID);
    
    // ✅ NOVO: Atualizar ícones de todos os slots
    UpdateAllSlotIcons();
}

void USkillWidget::OpenSkillDropdown(const FName& SlotID)
{
    // TODO: Criar e mostrar o widget dropdown
    // 1. Criar widget dropdown
    // 2. Posicionar próximo ao slot
    // 3. Mostrar lista de habilidades
    
    if (IsControllerValid())
    {
        TArray<FName> UnlockedSkills = SkillTreeController->GetUnlockedSkills();
        
        // TODO: Criar dropdown widget com essas habilidades
    }
}

void USkillWidget::CloseSkillDropdown()
{
    // TODO: Fechar o dropdown atual
}

void USkillWidget::EquipSkill(const FName& SkillID)
{
    // TODO: Implementar equipamento de habilidade
    // 1. Verificar se o controller é válido
    // 2. Equipar a habilidade no slot atual
    // 3. Atualizar visual
    
    if (IsControllerValid())
    {
        // TODO: Implementar lógica de equipamento
    }
}

void USkillWidget::EquipSkillInSlot(const FName& SkillID, const FName& SlotID)
{
    if (!IsControllerValid())
    {
        return;
    }
    
    // ✅ NOVO: Obter a InputTag do SlotSkillWidget
    FGameplayTag SlotInputTag = GetSlotInputTag(SlotID);
    
    // Obter o personagem atual
    ARPGCharacter* Character = SkillTreeController->GetCurrentCharacter();
    if (!Character)
    {
        return;
    }
    
    // Obter o SkillEquipmentComponent
    USkillEquipmentComponent* SkillEquipComponent = Character->GetSkillEquipmentComponent();
    if (!SkillEquipComponent)
    {
        return;
    }
    
    // ✅ NOVO: Equipar a habilidade no slot usando a InputTag do slot
    bool bSuccess = SkillEquipComponent->EquipSkill(SkillID, SlotID, SlotInputTag);
    
    if (bSuccess)
    {
        // Disparar delegate de habilidade equipada
        OnSkillWidgetSkillEquipped.Broadcast(SlotID, SkillID);
        
        // ✅ NOVO: Chamar OnSkillEquipped para atualizar ícones
        OnSkillEquipped(SlotID, SkillID);
        
        // Atualizar visual dos slots
        UpdateWidgetFromController();
    }
    
}

void USkillWidget::UnequipSkillFromSlot(const FName& SlotID)
{
    if (!IsControllerValid())
    {
        return;
    }
    
    // Obter o personagem atual
    ARPGCharacter* Character = SkillTreeController->GetCurrentCharacter();
    if (!Character)
    {
        return;
    }
    
    // Obter o SkillEquipmentComponent
    USkillEquipmentComponent* SkillEquipComponent = Character->GetSkillEquipmentComponent();
    if (!SkillEquipComponent)
    {
        return;
    }
    
    // Obter a habilidade equipada no slot
    FName EquippedSkillID = SkillEquipComponent->GetSkillEquippedSlot(SlotID);
    
    if (EquippedSkillID.IsNone())
    {
        return;
    }
    
    // Desequipar a habilidade do slot
    bool bSuccess = SkillEquipComponent->UnequipSkill(SlotID);
    
    if (bSuccess)
    {
        // Disparar delegate de habilidade desequipada
        OnSkillWidgetSkillUnequipped.Broadcast(SlotID, EquippedSkillID);
        
        // ✅ NOVO: Chamar OnSkillUnequipped para atualizar ícones
        OnSkillUnequipped(SlotID, EquippedSkillID);
        
        // Atualizar visual dos slots
        UpdateWidgetFromController();
    }
}

// === HELPERS ===

bool USkillWidget::IsControllerValid() const
{
    return SkillTreeController != nullptr && IsValid(SkillTreeController);
}

void USkillWidget::UpdateWidgetFromController()
{
    if (!IsControllerValid())
    {
        return;
    }
    
    // Aqui você pode atualizar o widget com dados do controller
    // Por exemplo: atualizar lista de slots, habilidades equipadas, etc.
    
    TArray<FName> EquippedSkills = SkillTreeController->GetEquippedSkills();
}

void USkillWidget::ConnectSlot(USlotSkillWidget* SlotWidget)
{
    if (!SlotWidget)
    {
        return;
    }
    
    // Conectar eventos do slot usando UFUNCTION
    // Nota: Os delegates precisam ser conectados via Blueprint ou usando UFUNCTION
}

void USkillWidget::DisconnectSlot(USlotSkillWidget* SlotWidget)
{
    if (!SlotWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("🎮 SkillWidget::DisconnectSlot - SlotWidget é nulo"));
        return;
    }
    
    // Desconectar eventos do slot
    // Nota: Os delegates precisam ser desconectados via Blueprint ou usando UFUNCTION
} 

// === PERSONAGEM ===

ARPGCharacter* USkillWidget::GetCharacterByID(const FName& CharacterID) const
{
    // Buscar o personagem pelo CharacterID no CharacterMap
    for (const auto& Pair : CharacterMap)
    {
        if (Pair.Value && Pair.Value->GetCharacterUniqueID() == CharacterID)
        {
            return Pair.Value;
        }
    }
    
    return nullptr;
}

void USkillWidget::OnCharacterSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    // Apenas processar se a seleção foi feita pelo usuário
    if (SelectionType != ESelectInfo::OnMouseClick && SelectionType != ESelectInfo::OnKeyPress)
    {
        return;
    }
    
    // Encontrar o personagem selecionado
    ARPGCharacter* SelectedCharacter = CharacterMap.FindRef(SelectedItem);
    if (SelectedCharacter)
    {
        CurrentCharacter = SelectedCharacter;
        
        // ATUALIZAR CONTROLLER COM O NOVO PERSONAGEM SELECIONADO
        if (SkillTreeController)
        {
            SkillTreeController->InitializeWithCharacter(CurrentCharacter);
        }
        
        // Obter CharacterID do personagem selecionado
        FName CharacterID = SelectedCharacter->GetCharacterUniqueID();
        
        // Atualizar header
        UpdateHeader();
        
        // DISPARAR delegate que ENVIA CharacterID para Blueprint
        OnCharacterChanged.Broadcast(CharacterID);
        
        // ✅ NOVO: Atualizar ícones de todos os slots
        UpdateAllSlotIcons();
    }
}

void USkillWidget::InitializeCharacterSelector()
{
    if (!CharacterSelector)
    {
        return;
    }
    
    // Conectar o evento de mudança de seleção
    CharacterSelector->OnSelectionChanged.AddDynamic(this, &USkillWidget::OnCharacterSelectionChanged);
    
    // Limpar dados anteriores
    CharacterSelector->ClearOptions();
    CharacterMap.Empty();
    
    // Obter o subsistema de party
    UPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<UPartySubsystem>();
    if (!PartySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("🎮 SkillWidget::InitializeCharacterSelector - Falha ao obter PartySubsystem"));
        return;
    }
    
    // Obter o personagem ativo (controlado pelo player)
    ARPGCharacter* ActiveCharacter = PartySubsystem->GetActivePartyMember();
    
    // Obter todos os personagens da party
    TArray<ARPGCharacter*> PartyMembers = PartySubsystem->GetPartyMembers();
    UE_LOG(LogTemp, Log, TEXT("🎮 SkillWidget::InitializeCharacterSelector - Encontrados %d membros na party"), PartyMembers.Num());
    
    // Reorganizar array para colocar o personagem ativo primeiro
    TArray<ARPGCharacter*> OrderedPartyMembers;
    
    // Adicionar o personagem ativo primeiro (se existir)
    if (ActiveCharacter && PartyMembers.Contains(ActiveCharacter))
    {
        OrderedPartyMembers.Add(ActiveCharacter);
    }
    
    // Adicionar os outros personagens
    for (ARPGCharacter* Character : PartyMembers)
    {
        if (Character && Character != ActiveCharacter)
        {
            OrderedPartyMembers.Add(Character);
        }
    }
    
    // Adicionar cada personagem ao ComboBox na ordem correta
    for (ARPGCharacter* Character : OrderedPartyMembers)
    {
        if (Character)
        {
            FString CharacterName = Character->GetCharacterName();
            CharacterSelector->AddOption(CharacterName);
            CharacterMap.Add(CharacterName, Character);
        }
    }
    
    // Selecionar o primeiro personagem (que será sempre o ativo)
    if (CharacterSelector->GetOptionCount() > 0)
    {
        CharacterSelector->SetSelectedIndex(0);
        FString SelectedOption = CharacterSelector->GetSelectedOption();
        ARPGCharacter* SelectedCharacter = CharacterMap.FindRef(SelectedOption);
        CurrentCharacter = SelectedCharacter;
        
        // Inicializar o controller com o personagem selecionado
        if (SkillTreeController)
        {
            SkillTreeController->InitializeWithCharacter(CurrentCharacter);
        }
        
        // Atualizar a UI
        UpdateHeader();
        
                    // Disparar delegate para inicializar os slots com o personagem ativo
            if (CurrentCharacter)
            {
                FName CharacterID = CurrentCharacter->GetCharacterUniqueID();
                OnCharacterChanged.Broadcast(CharacterID);
                
                // ✅ NOVO: Atualizar ícones de todos os slots na inicialização
                UpdateAllSlotIcons();
            }
    }
}

void USkillWidget::UpdateHeader()
{
    if (!CurrentCharacter)
    {
        return;
    }
    
    // Atualizar informações do personagem no header
    // (Implementar conforme necessário)
} 

void USkillWidget::UpdateAllSlotIcons()
{
    // Obter personagem ativo
    ARPGCharacter* ActiveCharacter = GetCurrentCharacter();
    if (!ActiveCharacter)
    {
        return;
    }
    
    // Obter SkillEquipmentComponent
    USkillEquipmentComponent* SkillEquipmentComponent = ActiveCharacter->GetSkillEquipmentComponent();
    if (!SkillEquipmentComponent)
    {
        return;
    }
    
    // Atualizar ícones de todos os slots gerenciados
    for (USlotSkillWidget* SlotWidget : ManagedSlots)
    {
        if (SlotWidget)
        {
            FName SlotID = SlotWidget->GetSlotID();
            UTexture2D* SlotIcon = SkillEquipmentComponent->GetSlotIcon(SlotID);
            
            SlotWidget->SetSlotIcon(SlotIcon);
            
            UE_LOG(LogTemp, Log, TEXT("🎮 SkillWidget::UpdateAllSlotIcons - Slot %s atualizado"), *SlotID.ToString());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("🎮 SkillWidget::UpdateAllSlotIcons - Concluído"));
} 

FGameplayTag USkillWidget::GetSlotInputTag(const FName& SlotID) const
{
    // ✅ NOVO: Verificar cache primeiro (SUPER RÁPIDO)
    if (CachedSlotInputTags.Contains(SlotID))
    {
        FGameplayTag CachedTag = CachedSlotInputTags[SlotID];
        return CachedTag;
    }
    
    // Buscar o SlotSkillWidget correspondente ao SlotID
    for (USlotSkillWidget* SlotWidget : ManagedSlots)
    {
        if (SlotWidget && SlotWidget->GetSlotID() == SlotID)
        {
            FGameplayTag InputTag = SlotWidget->GetInputTag();
            
            // ✅ NOVO: Adicionar ao cache para próximas buscas
            const_cast<USkillWidget*>(this)->CachedSlotInputTags.Add(SlotID, InputTag);
            
            return InputTag;
        }
    }
    
    return FGameplayTag::EmptyTag;
}

void USkillWidget::PopulateInputTagCache()
{
    // Limpar cache atual
    CachedSlotInputTags.Empty();
    
    // Popular cache com todos os slots
    for (USlotSkillWidget* SlotWidget : ManagedSlots)
    {
        if (SlotWidget)
        {
            FName SlotID = SlotWidget->GetSlotID();
            FGameplayTag InputTag = SlotWidget->GetInputTag();
            
            CachedSlotInputTags.Add(SlotID, InputTag);
        }
    }
} 

 