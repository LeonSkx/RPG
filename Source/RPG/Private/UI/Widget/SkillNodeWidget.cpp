// Copyright Druid Mechanics

#include "UI/Widget/SkillNodeWidget.h"
#include "UI/Widget/SkillTreeContentWidget.h"
#include "UI/WidgetController/SkillTreeWidgetController.h"
#include "Character/RPGCharacter.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Party/PartySubsystem.h"
#include "Progression/SkillTreeSubsystem.h"
#include "Progression/SkillTreeTableRow.h"
#include "UI/Widget/SkillDisplayData.h"

void USkillNodeWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Conectar eventos dos botões
    if (UnlockButton)
    {
        UnlockButton->OnClicked.AddDynamic(this, &USkillNodeWidget::OnUnlockClicked);
    }
    
    // Inicializar automaticamente com personagem ativo
    InitializeNodeAuto();
}

void USkillNodeWidget::NativeDestruct()
{
    // Desconectar eventos
    if (UnlockButton)
    {
        UnlockButton->OnClicked.RemoveAll(this);
    }
    
    Super::NativeDestruct();
}

void USkillNodeWidget::InitializeNodeAuto()
{
    // Verificar se o SlotID foi configurado no Editor
    if (SlotID.IsNone())
    {
        return;
    }
    
    // Obter personagem ativo do PartySubsystem
    ARPGCharacter* TargetCharacter = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                TargetCharacter = PartySubsystem->GetActiveCharacter();
            }
        }
    }
    
    if (!TargetCharacter)
    {
        return;
    }
    
    // Criar controller automaticamente
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            Controller = NewObject<USkillTreeWidgetController>(this);
            
            if (Controller)
            {
                // Inicializar controller com o personagem ativo
                Controller->InitializeWithCharacter(TargetCharacter);
                
                // Obter CharacterID do personagem
                FName CharacterID = TargetCharacter->GetCharacterUniqueID();
                
                // Buscar habilidade pelo CharacterID e SlotID
                if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
                {
                    FSkillTreeTableRow SkillRow = SkillTreeSubsystem->GetSkillTableRowBySlot(CharacterID, SlotID);
                    
                    if (!SkillRow.SkillID.IsNone())
                    {
                        // Atualizar CalculatedSkillID com o valor encontrado
                        CalculatedSkillID = SkillRow.SkillID;
                        
                        // Atualizar visual
                        UpdateVisual();
                        
                        // Mostrar o node
                        SetVisibility(ESlateVisibility::Visible);
                    }
                    else
                    {
                        // Slot não encontrado - ficar invisível
                        SetVisibility(ESlateVisibility::Hidden);
                    }
                }
            }
        }
    }
}

void USkillNodeWidget::UpdateCharacterID(const FName& NewCharacterID)
{
    if (!Controller || SlotID.IsNone())
    {
        return;
    }
    
    // Buscar o personagem pelo CharacterID diretamente do PartySubsystem
    ARPGCharacter* TargetCharacter = nullptr;
    
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                // Buscar o personagem pelo CharacterID na party
                TArray<ARPGCharacter*> PartyMembers = PartySubsystem->GetPartyMembers();
                
                for (ARPGCharacter* Character : PartyMembers)
                {
                    if (Character && Character->GetCharacterUniqueID() == NewCharacterID)
                    {
                        TargetCharacter = Character;
                        break;
                    }
                }
            }
        }
    }
    
    if (TargetCharacter)
    {
        // Atualizar controller com o novo personagem
        Controller->InitializeWithCharacter(TargetCharacter);
        
        // Buscar habilidade pelo CharacterID e SlotID
        if (UWorld* World = GetWorld())
        {
            if (UGameInstance* GameInstance = World->GetGameInstance())
            {
                if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
                {
                    FSkillTreeTableRow SkillRow = SkillTreeSubsystem->GetSkillTableRowBySlot(NewCharacterID, SlotID);
                    
                    if (!SkillRow.SkillID.IsNone())
                    {
                        // Atualizar CalculatedSkillID com o valor encontrado
                        CalculatedSkillID = SkillRow.SkillID;
                        
                        // Atualizar visual completo
                        UpdateVisual();
                        
                        // Mostrar o node
                        SetVisibility(ESlateVisibility::Visible);
                    }
                    else
                    {
                        // Slot não encontrado - ficar invisível
                        SetVisibility(ESlateVisibility::Hidden);
                    }
                }
            }
        }
    }
}

void USkillNodeWidget::UpdateVisual()
{
    if (!Controller)
    {
        return;
    }
    
    // Atualizar estado do nó
    UpdateNodeState();
}



void USkillNodeWidget::OnUnlockClicked()
{
    // Temporariamente desabilitado - usar UnlockSkill() diretamente no Blueprint
    //UnlockSkill();
}

void USkillNodeWidget::UpdateNodeState()
{
    if (!Controller)
    {
        return;
    }
    
    bool bIsUnlocked = Controller->SkillUnlockedBySlot(SlotID);
    bool bCanUnlock = Controller->CanUnlockSkillBySlot(SlotID);
    bool bIsEquipped = Controller->SkillEquippedBySlot(SlotID);
    
    // Determinar o estado atual
    ESkillState CurrentState = ESkillState::Locked;
    if (bIsEquipped)
    {
        CurrentState = ESkillState::Equipped;
    }
    else if (bIsUnlocked)
    {
        CurrentState = ESkillState::Unlocked;
    }
    else if (bCanUnlock)
    {
        CurrentState = ESkillState::Unlocked;
    }
    
    // Disparar delegate com o estado atual
    NotifySkillStateChanged(CurrentState, bIsUnlocked, bIsEquipped, bCanUnlock);
    
    // Atualizar botão de desbloqueio - SEMPRE VISÍVEL
    if (UnlockButton)
    {
        UnlockButton->SetVisibility(ESlateVisibility::Visible);
    }
    
    // Atualizar ícone de status
    if (StatusIcon)
    {
        if (bIsEquipped)
        {
            // Verde - equipado
            StatusIcon->SetColorAndOpacity(FLinearColor::Green);
            StatusIcon->SetVisibility(ESlateVisibility::Visible);
        }
        else if (bIsUnlocked)
        {
            // Azul - desbloqueado (pode equipar)
            StatusIcon->SetColorAndOpacity(FLinearColor::Blue);
            StatusIcon->SetVisibility(ESlateVisibility::Visible);
        }
        else if (bCanUnlock)
        {
            // Amarelo - pode desbloquear
            StatusIcon->SetColorAndOpacity(FLinearColor::Yellow);
            StatusIcon->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            // Cinza - bloqueado
            StatusIcon->SetColorAndOpacity(FLinearColor::Gray);
            StatusIcon->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

FText USkillNodeWidget::GetSkillName() const
{
    if (CalculatedSkillID.IsNone())
    {
        return FText::FromString("Habilidade Desconhecida");
    }
    
    // Buscar o nome real da habilidade no SkillTreeSubsystem
    if (Controller)
    {
        if (USkillTreeSubsystem* SkillTreeSubsystem = Controller->GetSkillTreeSubsystem())
        {
            // Buscar no Data Table por CharacterID e SlotID
            FName CharacterID = NAME_None;
            if (Controller->GetCurrentCharacter())
            {
                CharacterID = Controller->GetCurrentCharacter()->GetCharacterUniqueID();
            }
            FSkillTreeTableRow SkillRow = SkillTreeSubsystem->GetSkillTableRowBySlot(CharacterID, SlotID);
            
            // Retornar o nome da habilidade (suporta localização)
            if (!SkillRow.SkillName.IsEmpty())
            {
                return SkillRow.SkillName;
            }
        }
    }
    
    // Fallback: retornar o SkillID como texto
    return FText::FromName(CalculatedSkillID);
}



FSkillDisplayData USkillNodeWidget::GetSkillDisplayData() const
{
    FSkillDisplayData DisplayData;
    
    if (CalculatedSkillID.IsNone())
    {
        DisplayData.SkillName = FText::FromString("Habilidade Desconhecida");
        DisplayData.SkillCost = 0;
        DisplayData.SkillDescription = FText::FromString("Descrição não disponível");
    }
    else
    {
        // Buscar dados reais da habilidade no SkillTreeSubsystem
        if (Controller)
        {
            if (USkillTreeSubsystem* SkillTreeSubsystem = Controller->GetSkillTreeSubsystem())
            {
                // Buscar no Data Table por CharacterID e SlotID
                FName CharacterID = NAME_None;
                if (Controller->GetCurrentCharacter())
                {
                    CharacterID = Controller->GetCurrentCharacter()->GetCharacterUniqueID();
                }
                FSkillTreeTableRow SkillRow = SkillTreeSubsystem->GetSkillTableRowBySlot(CharacterID, SlotID);
                
                // Nome da habilidade (suporta localização)
                DisplayData.SkillName = SkillRow.SkillName;
                DisplayData.SkillCost = SkillRow.RequiredSpellPoints;
                DisplayData.SkillDescription = SkillRow.SkillDescription;
                DisplayData.SkillIcon = SkillRow.SkillIcon;
                DisplayData.Category = SkillRow.Category;
                DisplayData.Prerequisites = SkillRow.Prerequisites;
                DisplayData.RequiredQuestID = SkillRow.RequiredQuestID;
            }
        }
        
        // Fallback para nome se não conseguir buscar
        if (DisplayData.SkillName.IsEmpty())
        {
            DisplayData.SkillName = FText::FromName(CalculatedSkillID);
        }
        
        // Fallback se não conseguir buscar
        if (DisplayData.SkillCost == 0)
        {
            DisplayData.SkillCost = 1; // Valor padrão
        }
        
        // Fallback para descrição
        if (DisplayData.SkillDescription.IsEmpty())
        {
            DisplayData.SkillDescription = FText::FromString("Uma habilidade mágica poderosa");
        }
        
        // Definir o estado da habilidade (3 estados)
        if (Controller)
        {
            bool bIsLocked = Controller->SkillLockedBySlot(SlotID);
            bool bCanUnlock = Controller->CanUnlockSkillBySlot(SlotID);
            bool bIsUnlocked = Controller->SkillUnlockedBySlot(SlotID);
            bool bIsEquipped = Controller->SkillEquippedBySlot(SlotID);
            
            // Definir todas as propriedades booleanas
            DisplayData.bLocked = bIsLocked;
            DisplayData.bUnlocked = bIsUnlocked;
            DisplayData.bEquipped = bIsEquipped;
            DisplayData.bCanUnlock = bCanUnlock;
            
            // Determinar o estado baseado nas condições
            if (bIsEquipped)
            {
                // Estado 3: EQUIPADA
                DisplayData.SkillState = ESkillState::Equipped;
            }
            else if (bIsUnlocked)
            {
                // Estado 2: DESBLOQUEADA (pode equipar)
                DisplayData.SkillState = ESkillState::Unlocked;
            }
            else if (bCanUnlock)
            {
                // Estado 2: PODE DESBLOQUEAR
                DisplayData.SkillState = ESkillState::Unlocked;
            }
            else
            {
                // Estado 1: BLOQUEADA
                DisplayData.SkillState = ESkillState::Locked;
            }
            
        }
        else
        {
            DisplayData.bLocked = true; // Se não tem controller, considera bloqueada
            DisplayData.bEquipped = false; // Se não tem controller, considera não equipada
            DisplayData.SkillState = ESkillState::Locked;
        }
    }
    
    return DisplayData;
}

bool USkillNodeWidget::UnlockSkill()
{
    if (!Controller)
    {
        return false;
    }
    
    // Tentar desbloquear a habilidade por slot
    bool bSuccess = Controller->UnlockSkillBySlot(SlotID);
    
    if (bSuccess)
    {
        // Atualizar visual após desbloqueio
        UpdateVisual();
    }
    
    return bSuccess;
}

bool USkillNodeWidget::EquipSkill()
{
    if (!Controller)
    {
        return false;
    }
    
    // Verificar se a habilidade está desbloqueada
    if (!Controller->SkillUnlockedBySlot(SlotID))
    {
        return false;
    }
    
    // Tentar equipar a habilidade por slot
    bool bSuccess = Controller->EquipSkillBySlot(SlotID);
    
    if (bSuccess)
    {
        // Atualizar visual após equipamento
        UpdateVisual();
    }
    
    return bSuccess;
}

bool USkillNodeWidget::UnequipSkill()
{
    if (!Controller)
    {
        return false;
    }
    
    // Verificar se a habilidade está equipada
    if (!Controller->SkillEquippedBySlot(SlotID))
    {
        return false;
    }
    
    // Tentar desequipar a habilidade por slot
    bool bSuccess = Controller->UnequipSkillBySlot(SlotID);
    
    if (bSuccess)
    {
        // Atualizar visual após desequipamento
        UpdateVisual();
    }
    
    return bSuccess;
}

void USkillNodeWidget::UnlockSkillsWithoutPrerequisites()
{
    if (!Controller)
    {
        return;
    }
    
    // Chamar a função do Controller
    Controller->UnlockSkillsWithoutPrerequisites();
    
    // Atualizar visual após desbloqueio
    UpdateVisual();
}

void USkillNodeWidget::SetController(USkillTreeWidgetController* InController)
{
    Controller = InController;
}

void USkillNodeWidget::NotifySkillStateChanged(ESkillState NewState, bool bIsUnlocked, bool bIsEquipped, bool bCanUnlock)
{
    // Disparar o delegate
    OnSkillStateChanged_Event.Broadcast(CalculatedSkillID, SlotID, NewState, bIsUnlocked, bIsEquipped, bCanUnlock);
}