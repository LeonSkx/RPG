// Copyright Druid Mechanics

#include "UI/Widget/SkillDropdownWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "UI/Widget/SkillWidget.h"
#include "UI/Widget/SkillItemWidget.h"
#include "Character/RPGCharacter.h"
#include "Progression/SkillTreeSubsystem.h"
#include "Progression/SkillTreeTableRow.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "UI/WidgetController/SkillTreeWidgetController.h"
#include "Party/PartySubsystem.h"
#include "Components/SkillEquipmentComponent.h"

void USkillDropdownWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Inicializar estado
    bIsVisible = false;
    bIsRefreshing = false; // ✅ NOVO: Flag para controlar refresh
    CurrentSelectedItem = nullptr;
    CurrentCharacterID = NAME_None; // ✅ NOVO: Inicializar como inválido
    
    // Configurar timer para atualizações periódicas
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RefreshTimerHandle,
            this,
            &USkillDropdownWidget::PeriodicRefresh,
            0.5f, // 500ms
            true
        );
    }
    
    // Não inicializar controller aqui - será feito quando SetTargetCharacterID for chamado
}

void USkillDropdownWidget::NativeDestruct()
{
    // Limpar timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RefreshTimerHandle);
    }
    
    // Desconectar eventos
    DisconnectFromSkillTreeEvents();
    
    // Limpar referências
    Controller = nullptr;
    ParentSkillWidget = nullptr;
    CurrentSelectedItem = nullptr;
    CurrentCharacterID = NAME_None;
    
    Super::NativeDestruct();
}

void USkillDropdownWidget::SetParentSkillWidget(USkillWidget* InSkillWidget)
{
    if (ParentSkillWidget != InSkillWidget)
    {
        ParentSkillWidget = InSkillWidget;
    }
}

void USkillDropdownWidget::ShowSkillList()
{
    // ✅ VALIDAÇÃO: Verificar se temos um CharacterID válido
    if (CurrentCharacterID.IsNone())
    {
        return;
    }
    
    if (!bIsVisible)
    {
        bIsVisible = true;
        
        // Inicializar/atualizar controller
        InitializeController();
        
        // Conectar aos eventos se necessário
        ConnectToSkillTreeEvents();
        
        // Atualizar lista
        RefreshSkillList();
        
        // Mostrar widget
        SetVisibility(ESlateVisibility::Visible);
        
            // ✅ NOVO: Disparar evento de visibilidade
    OnDropdownVisibilityChanged.Broadcast(true);
    
            // ✅ NOVO: Disparar evento de personagem atual
    OnDropdownCharacterChanged.Broadcast(CurrentCharacterID);
    }
}

void USkillDropdownWidget::HideSkillList()
{
    if (bIsVisible)
    {
        bIsVisible = false;
        
        // ✅ MELHORADO: Não limpar CurrentSelectedItem para manter estado
        // CurrentSelectedItem = nullptr;
        
        // Esconder widget
        SetVisibility(ESlateVisibility::Hidden);
        
        // ✅ NOVO: Disparar evento de visibilidade
        OnDropdownVisibilityChanged.Broadcast(false);
    }
}

void USkillDropdownWidget::EquipSkill(const FName& SkillID)
{
    if (ParentSkillWidget)
    {
        // Usar a nova função com SlotID
        ParentSkillWidget->EquipSkillInSlot(SkillID, CurrentSlotID);
        
        // Esconder lista após equipar
        HideSkillList();
    }
}

void USkillDropdownWidget::SetCurrentSlotID(const FName& SlotID)
{
    // ✅ IMPLEMENTADO: Lógica de comparação de slots
    // Se lista está visível e mudou de slot, fechar e reabrir
    if (bIsVisible && CurrentSlotID != SlotID)
    {
        // Fechar lista atual
        HideSkillList();
        
        // Atualizar slot
        CurrentSlotID = SlotID;
        
        // Reabrir lista com novo slot
        ShowSkillList();
    }
    else if (!bIsVisible)
    {
        // Lista não está visível, apenas definir slot e abrir
        CurrentSlotID = SlotID;
        ShowSkillList();
    }
    else
    {
        // Mesmo slot, apenas atualizar (não fazer nada visual)
        CurrentSlotID = SlotID;
    }
}

void USkillDropdownWidget::ClearSkillList()
{
    if (SkillListScrollBox)
    {
        SkillListScrollBox->ClearChildren();
        // ✅ MELHORADO: Não resetar seleção aqui para manter estado
        // CurrentSelectedItem = nullptr;
    }
}

void USkillDropdownWidget::InitializeController()
{
    // ✅ CORRIGIDO: Usar sempre o personagem do CurrentCharacterID
    ARPGCharacter* TargetCharacter = GetCharacterByID(CurrentCharacterID);
    if (!TargetCharacter)
    {
        return;
    }
    
    // Se já temos um controller válido para o personagem atual, não recriar
    if (Controller && Controller->GetCurrentCharacter() == TargetCharacter)
    {
        return;
    }
    
    // Desconectar eventos do controller anterior
    DisconnectFromSkillTreeEvents();
    
    // Criar novo controller
    Controller = NewObject<USkillTreeWidgetController>(this);
    if (Controller)
    {
        Controller->InitializeWithCharacter(TargetCharacter);
    }
}

// ✅ REMOVIDO: GetActiveCharacter() - não é mais usado

TArray<FName> USkillDropdownWidget::GetUnlockedSkills()
{
    TArray<FName> UnlockedSkills;
    
    if (!Controller)
    {
        return UnlockedSkills;
    }
    
    // ✅ CORRIGIDO: Usar sempre o personagem do CurrentCharacterID
    ARPGCharacter* TargetCharacter = GetCharacterByID(CurrentCharacterID);
    if (!TargetCharacter)
    {
        return UnlockedSkills;
    }
    
    // Obter skills desbloqueadas via Controller
    UnlockedSkills = Controller->GetUnlockedSkills();
    
    // Filtrar skills equipadas (não mostrar na lista)
    USkillEquipmentComponent* SkillEquipComponent = TargetCharacter->GetSkillEquipmentComponent();
    if (SkillEquipComponent)
    {
        TMap<FName, FName> EquippedSkills = SkillEquipComponent->GetAllEquippedSkills();
        for (const auto& EquippedPair : EquippedSkills)
        {
            UnlockedSkills.Remove(EquippedPair.Value);
        }
    }
    
    return UnlockedSkills;
}

void USkillDropdownWidget::CreateSkillItemWidgets(const TArray<FName>& SkillIDs)
{
    if (!SkillListScrollBox || !SkillItemWidgetClass)
    {
        return;
    }
    
    if (SkillIDs.Num() == 0)
    {
        CreateEmptyListMessage();
        return;
    }
    
    // ✅ CORRIGIDO: Usar sempre o personagem do CurrentCharacterID
    ARPGCharacter* TargetCharacter = GetCharacterByID(CurrentCharacterID);
    if (!TargetCharacter)
    {
        return;
    }
    
    for (const FName& SkillID : SkillIDs)
    {
        if (USkillItemWidget* SkillItemWidget = CreateWidget<USkillItemWidget>(this, SkillItemWidgetClass))
        {
            // Obter dados da habilidade
            FSkillDisplayData SkillData = GetSkillDisplayData(SkillID);
            
            // Configurar o widget
            SkillItemWidget->SetSkillData(
                SkillID, 
                SkillData.SkillName, 
                SkillData.SkillIcon, 
                false // Nunca equipada na lista (já filtradas)
            );
            
            SkillItemWidget->SetParentDropdown(this);
            SkillListScrollBox->AddChild(SkillItemWidget);
        }
    }
}

FSkillDisplayData USkillDropdownWidget::GetSkillDisplayData(const FName& SkillID)
{
    FSkillDisplayData SkillData;
    
    // Dados padrão
    SkillData.SkillName = FText::FromName(SkillID);
    SkillData.SkillIcon = nullptr;
    SkillData.bEquipped = false;
    
    if (!Controller)
    {
        return SkillData;
    }
    
    // ✅ CORRIGIDO: Usar sempre o personagem do CurrentCharacterID
    ARPGCharacter* TargetCharacter = GetCharacterByID(CurrentCharacterID);
    if (!TargetCharacter)
    {
        return SkillData;
    }
    
    // Usar o SkillTreeSubsystem para obter dados reais
    if (USkillTreeSubsystem* SkillTreeSubsystem = Controller->GetSkillTreeSubsystem())
    {
        FName CharacterID = TargetCharacter->GetCharacterUniqueID();
        
        // Buscar na tabela de dados do personagem
        UDataTable* CharacterDataTable = SkillTreeSubsystem->GetCharacterSkillTable(CharacterID);
        if (CharacterDataTable)
        {
            TArray<FName> RowNames = CharacterDataTable->GetRowNames();
            
            for (const FName& RowName : RowNames)
            {
                if (FSkillTreeTableRow* Row = CharacterDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GetSkillDisplayData")))
                {
                    if (Row->SkillID == SkillID)
                    {
                        SkillData.SkillName = Row->SkillName;
                        SkillData.SkillIcon = Row->SkillIcon;
                        SkillData.SkillDescription = Row->SkillDescription;
                        SkillData.SkillCost = Row->RequiredSpellPoints;
                        SkillData.Category = Row->Category;
                        SkillData.RequiredQuestID = Row->RequiredQuestID;
                        break;
                    }
                }
            }
        }
        
        // Verificar estados via Controller
        if (!SkillData.SkillName.IsEmpty())
        {
            // Buscar SlotID para verificar estados
            FName SlotID = FindSlotIDForSkill(SkillID);
            if (!SlotID.IsNone())
            {
                SkillData.bLocked = Controller->SkillLockedBySlot(SlotID);
                SkillData.bUnlocked = Controller->SkillUnlockedBySlot(SlotID);
                SkillData.bEquipped = Controller->SkillEquippedBySlot(SlotID);
                SkillData.bCanUnlock = Controller->CanUnlockSkillBySlot(SlotID);
            }
        }
    }
    
    // Fallbacks
    if (SkillData.SkillName.IsEmpty())
    {
        SkillData.SkillName = FText::FromString(FString::Printf(TEXT("Skill %s"), *SkillID.ToString()));
    }
    
    return SkillData;
}

FName USkillDropdownWidget::FindSlotIDForSkill(const FName& SkillID)
{
    if (!Controller)
    {
        return NAME_None;
    }
    
    // ✅ CORRIGIDO: Usar sempre o personagem do CurrentCharacterID
    ARPGCharacter* TargetCharacter = GetCharacterByID(CurrentCharacterID);
    if (!TargetCharacter)
    {
        return NAME_None;
    }
    
    if (USkillTreeSubsystem* SkillTreeSubsystem = Controller->GetSkillTreeSubsystem())
    {
        FName CharacterID = TargetCharacter->GetCharacterUniqueID();
        UDataTable* CharacterDataTable = SkillTreeSubsystem->GetCharacterSkillTable(CharacterID);
        
        if (CharacterDataTable)
        {
            TArray<FName> RowNames = CharacterDataTable->GetRowNames();
            
            for (const FName& RowName : RowNames)
            {
                if (FSkillTreeTableRow* Row = CharacterDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("FindSlotIDForSkill")))
                {
                    if (Row->SkillID == SkillID)
                    {
                        return Row->SlotID;
                    }
                }
            }
        }
    }
    
    return NAME_None;
}

void USkillDropdownWidget::SetTargetCharacterID(const FName& CharacterID)
{
    // ✅ MELHORADO: Validar se o personagem existe ANTES de definir
    ARPGCharacter* TargetCharacter = GetCharacterByID(CharacterID);
    if (!TargetCharacter)
    {
        return;
    }
    
    // Verificar se o CharacterID mudou
    if (CurrentCharacterID == CharacterID)
    {
        return;
    }
    
    // Atualizar CharacterID interno
    CurrentCharacterID = CharacterID;
    
    // Reinicializar controller com o novo personagem
    InitializeController();
    
    // Reconectar eventos
    ConnectToSkillTreeEvents();
    
    // Recarregar lista de skills se estiver visível
    if (bIsVisible)
    {
        RefreshSkillList();
    }
    
    // ✅ NOVO: Disparar evento de mudança de personagem
    OnDropdownCharacterChanged.Broadcast(CurrentCharacterID);
}

ARPGCharacter* USkillDropdownWidget::GetCharacterByID(const FName& CharacterID) const
{
    // ✅ VALIDAÇÃO: Verificar se CharacterID é válido
    if (CharacterID.IsNone())
    {
        return nullptr;
    }
    
    // Buscar o personagem pelo CharacterID no PartySubsystem
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                TArray<ARPGCharacter*> PartyMembers = PartySubsystem->GetPartyMembers();
                
                for (ARPGCharacter* Character : PartyMembers)
                {
                    if (Character && Character->GetCharacterUniqueID() == CharacterID)
                    {
                        return Character;
                    }
                }
            }
        }
    }
    
    return nullptr;
}

void USkillDropdownWidget::RefreshSkillList()
{
    // ✅ MELHORADO: Evitar refresh concorrente
    if (bIsRefreshing)
    {
        return;
    }
    
    // ✅ VALIDAÇÃO: Verificar se temos um CharacterID válido
    if (CurrentCharacterID.IsNone())
    {
        return;
    }
    
    bIsRefreshing = true;
    
    // Limpar lista atual
    ClearSkillList();
    
    // Obter skills do personagem atual
    TArray<FName> UnlockedSkills = GetUnlockedSkills();
    
    // Criar widgets para as skills
    CreateSkillItemWidgets(UnlockedSkills);
    
    bIsRefreshing = false;
}

void USkillDropdownWidget::CreateEmptyListMessage()
{
    if (SkillListScrollBox)
    {
        UTextBlock* EmptyText = NewObject<UTextBlock>(this);
        if (EmptyText)
        {
            EmptyText->SetText(FText::FromString(TEXT("Nenhuma habilidade disponível")));
            EmptyText->SetJustification(ETextJustify::Center);
            
            // Aplicar estilo se disponível
            if (EmptyTextStyle.Font.HasValidFont())
            {
                EmptyText->SetFont(EmptyTextStyle.Font);
            }
            EmptyText->SetColorAndOpacity(FSlateColor(FLinearColor::Gray));
            
            SkillListScrollBox->AddChild(EmptyText);
            
        }
    }
}

void USkillDropdownWidget::PeriodicRefresh()
{
    // ✅ MELHORADO: Só atualizar se estiver visível E tiver CharacterID válido
    if (bIsVisible && !CurrentCharacterID.IsNone() && !bIsRefreshing)
    {
        // Verificar se o personagem ainda existe
        ARPGCharacter* TargetCharacter = GetCharacterByID(CurrentCharacterID);
        if (!TargetCharacter)
        {
            HideSkillList();
            return;
        }
        
        // Verificar se o controller precisa ser reinicializado
        if (!Controller || Controller->GetCurrentCharacter() != TargetCharacter)
        {
            InitializeController();
            ConnectToSkillTreeEvents();
            RefreshSkillList();
        }
    }
}

void USkillDropdownWidget::ConnectToSkillTreeEvents()
{
    if (!Controller)
    {
        return;
    }
    
    if (USkillTreeSubsystem* SkillTreeSubsystem = Controller->GetSkillTreeSubsystem())
    {
        // ✅ CORRIGIDO: Remover verificação IsAlreadyBound que estava causando erro
        // Desconectar eventos antigos primeiro (por segurança)
        SkillTreeSubsystem->OnSkillUnlocked.RemoveAll(this);
        
        // Conectar apenas ao evento de desbloqueio (equipamento movido para SkillEquipmentComponent)
        SkillTreeSubsystem->OnSkillUnlocked.AddDynamic(this, &USkillDropdownWidget::OnSkillUnlocked);
    }
}

void USkillDropdownWidget::DisconnectFromSkillTreeEvents()
{
    if (Controller)
    {
        if (USkillTreeSubsystem* SkillTreeSubsystem = Controller->GetSkillTreeSubsystem())
        {
            SkillTreeSubsystem->OnSkillUnlocked.RemoveAll(this);
        }
    }
}

void USkillDropdownWidget::OnSkillUnlocked(ARPGCharacter* Character, FName SkillID)
{
    // ✅ CORRIGIDO: Usar CurrentCharacterID em vez de personagem ativo
    if (Character && Character->GetCharacterUniqueID() == CurrentCharacterID && bIsVisible)
    {
        RefreshSkillList();
    }
}

// REMOVIDO: Eventos de equipamento movidos para SkillEquipmentComponent

void USkillDropdownWidget::OnSkillItemSelected(USkillItemWidget* SelectedItem)
{
    if (!SelectedItem)
    {
        return;
    }
    
    // Se clicou no mesmo item que já estava selecionado, equipar
    if (CurrentSelectedItem == SelectedItem && SelectedItem->IsSelected())
    {
        FName SkillID = SelectedItem->GetSkillID();
        if (!SkillID.IsNone())
        {
            EquipSkill(SkillID);
        }
        return;
    }
    
    // Primeiro clique: selecionar o item
    // Desselecionar item anteriormente selecionado
    if (CurrentSelectedItem && CurrentSelectedItem != SelectedItem)
    {
        CurrentSelectedItem->SetSelected(false);
    }
    
    // Atualizar seleção atual e marcar como selecionado
    CurrentSelectedItem = SelectedItem;
    SelectedItem->SetSelected(true);
}

