// Copyright Druid Mechanics

#include "UI/WidgetController/SkillTreeWidgetController.h"
#include "Character/RPGCharacter.h"
#include "Components/SkillEquipmentComponent.h"
#include "Progression/SkillTreeSubsystem.h"
#include "Progression/SkillTreeTableRow.h"
#include "Interaction/PlayerInterface.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

USkillTreeWidgetController::USkillTreeWidgetController()
    : CurrentCharacter(nullptr)
    , SkillTreeSubsystem(nullptr)
    , SkillEquipmentComponent(nullptr)
    , CharacterSkillDataTable(nullptr)
{
}

// === INICIALIZAÇÃO ===

void USkillTreeWidgetController::InitializeWithCharacter(ARPGCharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    // Desconectar do personagem anterior
    DisconnectFromCharacterEvents();
    
    // Definir novo personagem
    CurrentCharacter = Character;
    
    // Obter SkillTreeSubsystem
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>();
        }
    }
    
    // Obter SkillEquipmentComponent
    SkillEquipmentComponent = Character->FindComponentByClass<USkillEquipmentComponent>();
    
    // Atualizar Data Table
    UpdateSkillDataTable();
    
    // Conectar aos eventos do personagem
    ConnectToCharacterEvents();
}

void USkillTreeWidgetController::UpdateCharacter(ARPGCharacter* NewCharacter)
{
    InitializeWithCharacter(NewCharacter);
}

// === HABILIDADES ===

bool USkillTreeWidgetController::CanUnlockSkill(const FName& SkillID) const
{
    if (!ValidateCurrentCharacter() || !SkillTreeSubsystem)
    {
        return false;
    }
    
    return SkillTreeSubsystem->CanUnlockSkill(CurrentCharacter, SkillID);
}

bool USkillTreeWidgetController::CanUnlockSkillBySlot(const FName& SlotID) const
{
    if (!ValidateCurrentCharacter() || !SkillTreeSubsystem)
    {
        return false;
    }
    
    return SkillTreeSubsystem->CanUnlockSkillBySlot(CurrentCharacter, SlotID);
}

bool USkillTreeWidgetController::UnlockSkill(const FName& SkillID)
{
    if (!ValidateCurrentCharacter() || !SkillTreeSubsystem)
    {
        return false;
    }
    
    return SkillTreeSubsystem->UnlockSkill(CurrentCharacter, SkillID);
}

bool USkillTreeWidgetController::UnlockSkillBySlot(const FName& SlotID)
{
    if (!ValidateCurrentCharacter() || !SkillTreeSubsystem)
    {
        return false;
    }
    
    return SkillTreeSubsystem->UnlockSkillBySlot(CurrentCharacter, SlotID);
}

bool USkillTreeWidgetController::SkillUnlocked(const FName& SkillID) const
{
    if (!ValidateCurrentCharacter() || !SkillTreeSubsystem)
    {
        return false;
    }
    
    return SkillTreeSubsystem->SkillUnlocked(CurrentCharacter, SkillID);
}

bool USkillTreeWidgetController::SkillUnlockedBySlot(const FName& SlotID) const
{
    if (!ValidateCurrentCharacter() || !SkillTreeSubsystem)
    {
        return false;
    }
    
    return SkillTreeSubsystem->SkillUnlockedBySlot(CurrentCharacter, SlotID);
}

FSkillTreeNode USkillTreeWidgetController::GetSkillNode(const FName& SkillID) const
{
    if (!ValidateCurrentCharacter() || !SkillTreeSubsystem)
    {
        return FSkillTreeNode();
    }
    
    FName CharacterID = CurrentCharacter->GetCharacterUniqueID();
    return SkillTreeSubsystem->GetSkillData(CharacterID, SkillID);
}

TArray<FSkillTreeNode> USkillTreeWidgetController::GetAllAvailableSkills() const
{
    TArray<FSkillTreeNode> AvailableSkills;
    
    if (!ValidateCurrentCharacter() || !CharacterSkillDataTable)
    {
        return AvailableSkills;
    }
    
    // Buscar todas as habilidades na tabela
    TArray<FName> RowNames = CharacterSkillDataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        // Aqui você pode implementar a lógica para obter dados da tabela
        // Por enquanto, retornamos uma lista vazia
    }
    
    return AvailableSkills;
}

TArray<FName> USkillTreeWidgetController::GetUnlockedSkills() const
{
    if (!ValidateCurrentCharacter() || !SkillTreeSubsystem)
    {
        return TArray<FName>();
    }
    
    return SkillTreeSubsystem->GetUnlockedSkills(CurrentCharacter);
}

TArray<FSkillTreeNode> USkillTreeWidgetController::GetUnlockableSkills() const
{
    TArray<FSkillTreeNode> UnlockableSkills;
    
    if (!ValidateCurrentCharacter() || !SkillTreeSubsystem)
    {
        return UnlockableSkills;
    }
    
    // Obter todas as habilidades disponíveis
    TArray<FSkillTreeNode> AllSkills = GetAllAvailableSkills();
    
    // Filtrar apenas as que podem ser desbloqueadas
    for (const FSkillTreeNode& SkillNode : AllSkills)
    {
        if (!SkillTreeSubsystem->SkillUnlocked(CurrentCharacter, SkillNode.SkillID) &&
            SkillTreeSubsystem->CanUnlockSkill(CurrentCharacter, SkillNode.SkillID))
        {
            UnlockableSkills.Add(SkillNode);
        }
    }
    
    return UnlockableSkills;
}

// === EQUIPAMENTO ===

bool USkillTreeWidgetController::CanEquipSkill(const FName& SkillID, const FName& SlotID) const
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return false;
    }
    
    return SkillEquipmentComponent->CanEquipSkill(SkillID, SlotID);
}

bool USkillTreeWidgetController::EquipSkill(const FName& SkillID, const FName& SlotID, const FGameplayTag& SlotInputTag)
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return false;
    }
    
    return SkillEquipmentComponent->EquipSkill(SkillID, SlotID, SlotInputTag);
}

bool USkillTreeWidgetController::UnequipSkill(const FName& SlotID)
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return false;
    }
    
    return SkillEquipmentComponent->UnequipSkill(SlotID);
}

bool USkillTreeWidgetController::MoveSkillToSlot(const FName& FromSlotID, const FName& ToSlotID)
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return false;
    }
    
    return SkillEquipmentComponent->MoveSkillToSlot(FromSlotID, ToSlotID);
}

bool USkillTreeWidgetController::SwapSkillsBetweenSlots(const FName& SlotID1, const FName& SlotID2)
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return false;
    }
    
    return SkillEquipmentComponent->SwapSkillsBetweenSlots(SlotID1, SlotID2);
}

FName USkillTreeWidgetController::GetEquippedSkillInSlot(const FName& SlotID) const
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return NAME_None;
    }
    
    return SkillEquipmentComponent->GetEquippedSkillInSlot(SlotID);
}

FName USkillTreeWidgetController::GetSkillEquippedSlot(const FName& SkillID) const
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return NAME_None;
    }
    
    return SkillEquipmentComponent->GetSkillEquippedSlot(SkillID);
}

TMap<FName, FName> USkillTreeWidgetController::GetAllEquippedSkills() const
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return TMap<FName, FName>();
    }
    
    return SkillEquipmentComponent->GetAllEquippedSkills();
}

// === PONTOS ===

int32 USkillTreeWidgetController::GetSpellPoints() const
{
    if (!ValidateCurrentCharacter())
    {
        return 0;
    }
    
    // Usar PlayerInterface diretamente (que chama ProgressionSubsystem)
    if (CurrentCharacter->GetClass()->ImplementsInterface(UPlayerInterface::StaticClass()))
    {
        return IPlayerInterface::Execute_GetSpellPoints(CurrentCharacter);
    }
    
    return 0;
}

void USkillTreeWidgetController::AddSpellPoints(int32 Points)
{
    if (!ValidateCurrentCharacter())
    {
        return;
    }
    
    // Usar PlayerInterface diretamente (que chama ProgressionSubsystem)
    if (CurrentCharacter->GetClass()->ImplementsInterface(UPlayerInterface::StaticClass()))
    {
        IPlayerInterface::Execute_AddToSpellPoints(CurrentCharacter, Points);
    }
}

bool USkillTreeWidgetController::SpendSpellPoints(int32 Points)
{
    if (!ValidateCurrentCharacter())
    {
        return false;
    }
    
    // Usar PlayerInterface diretamente (que chama ProgressionSubsystem)
    if (CurrentCharacter->GetClass()->ImplementsInterface(UPlayerInterface::StaticClass()))
    {
        // Verificar se tem pontos suficientes
        int32 CurrentPoints = IPlayerInterface::Execute_GetSpellPoints(CurrentCharacter);
        if (CurrentPoints >= Points)
        {
            IPlayerInterface::Execute_AddToSpellPoints(CurrentCharacter, -Points);
            return true;
        }
    }
    
    return false;
}

// === SLOTS ===

TArray<FName> USkillTreeWidgetController::GetActiveSlotIDs() const
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return TArray<FName>();
    }
    
    return SkillEquipmentComponent->GetActiveSlotIDs();
}

bool USkillTreeWidgetController::HasSlot(const FName& SlotID) const
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return false;
    }
    
    return SkillEquipmentComponent->HasSlot(SlotID);
}

int32 USkillTreeWidgetController::GetActiveSlotCount() const
{
    if (!ValidateCurrentCharacter() || !SkillEquipmentComponent)
    {
        return 0;
    }
    
    return SkillEquipmentComponent->GetActiveSlotCount();
}

// === UTILIDADES ===

UDataTable* USkillTreeWidgetController::GetSkillDataTable() const
{
    return CharacterSkillDataTable;
}

// === HELPERS ===

void USkillTreeWidgetController::ConnectToCharacterEvents()
{
    if (!CurrentCharacter)
    {
        return;
    }
    
    // Conectar ao SkillTreeSubsystem
    if (SkillTreeSubsystem)
    {
        SkillTreeSubsystem->OnSkillUnlocked.AddDynamic(this, &USkillTreeWidgetController::OnSkillUnlocked);
        SkillTreeSubsystem->OnPointsChanged.AddDynamic(this, &USkillTreeWidgetController::OnSpellPointsChanged);
    }
    
    // Conectar ao SkillEquipmentComponent
    if (SkillEquipmentComponent)
    {
        SkillEquipmentComponent->OnSkillEquipmentEquipped.AddDynamic(this, &USkillTreeWidgetController::OnSkillEquipped);
        SkillEquipmentComponent->OnSkillEquipmentUnequipped.AddDynamic(this, &USkillTreeWidgetController::OnSkillUnequipped);
    }
}

void USkillTreeWidgetController::DisconnectFromCharacterEvents()
{
    // Desconectar do SkillTreeSubsystem
    if (SkillTreeSubsystem)
    {
        SkillTreeSubsystem->OnSkillUnlocked.RemoveAll(this);
        SkillTreeSubsystem->OnPointsChanged.RemoveAll(this);
    }
    
    // Desconectar do SkillEquipmentComponent
    if (SkillEquipmentComponent)
    {
        SkillEquipmentComponent->OnSkillEquipmentEquipped.RemoveAll(this);
        SkillEquipmentComponent->OnSkillEquipmentUnequipped.RemoveAll(this);
    }
}

void USkillTreeWidgetController::OnSkillUnlocked(ARPGCharacter* Character, FName SkillID)
{
    // Verificar se é o nosso personagem
    if (Character == CurrentCharacter)
    {
        // Aqui você pode implementar lógica adicional quando uma habilidade é desbloqueada
        // Por exemplo: atualizar UI, mostrar notificação, etc.
    }
}

void USkillTreeWidgetController::OnSkillEquipped(FName CharacterID, FName SlotID, FName SkillID)
{
    // Verificar se é o nosso personagem
    if (CurrentCharacter && CurrentCharacter->GetCharacterUniqueID() == CharacterID)
    {
        // Aqui você pode implementar lógica adicional quando uma habilidade é equipada
        // Por exemplo: atualizar UI, mostrar notificação, etc.
    }
}

void USkillTreeWidgetController::OnSkillUnequipped(FName CharacterID, FName SlotID, FName SkillID)
{
    // Verificar se é o nosso personagem
    if (CurrentCharacter && CurrentCharacter->GetCharacterUniqueID() == CharacterID)
    {
        // Aqui você pode implementar lógica adicional quando uma habilidade é desequipada
        // Por exemplo: atualizar UI, mostrar notificação, etc.
    }
}

void USkillTreeWidgetController::OnSpellPointsChanged()
{
    // Aqui você pode implementar lógica adicional quando os pontos de magia mudam
    // Por exemplo: atualizar UI, mostrar notificação, etc.
}

void USkillTreeWidgetController::UpdateSkillDataTable()
{
    if (!CurrentCharacter || !SkillTreeSubsystem)
    {
        CharacterSkillDataTable = nullptr;
        return;
    }
    
    FName CharacterID = CurrentCharacter->GetCharacterUniqueID();
    CharacterSkillDataTable = SkillTreeSubsystem->GetCharacterSkillTable(CharacterID);
}

bool USkillTreeWidgetController::ValidateCurrentCharacter() const
{
    return CurrentCharacter != nullptr && CurrentCharacter->IsValidLowLevel();
}

// === MÉTODOS ADICIONAIS DO LEGACY ===

bool USkillTreeWidgetController::SkillLockedBySlot(const FName& SlotID) const
{
    if (!SkillTreeSubsystem || !CurrentCharacter)
    {
        return true;
    }
    
    return SkillTreeSubsystem->SkillLockedBySlot(CurrentCharacter, SlotID);
}

bool USkillTreeWidgetController::SkillEquippedBySlot(const FName& SlotID) const
{
    if (!SkillEquipmentComponent)
    {
        return false;
    }
    
    FName EquippedSkillID = SkillEquipmentComponent->GetSkillEquippedSlot(SlotID);
    return !EquippedSkillID.IsNone();
}

int32 USkillTreeWidgetController::GetCharacterLevel() const
{
    if (!CurrentCharacter)
    {
        return 1;
    }
    
    if (CurrentCharacter->GetClass()->ImplementsInterface(UPlayerInterface::StaticClass()))
    {
        int32 XP = IPlayerInterface::Execute_GetXP(CurrentCharacter);
        return IPlayerInterface::Execute_FindLevelForXP(CurrentCharacter, XP);
    }
    
    return 1;
}

FString USkillTreeWidgetController::GetCharacterName() const
{
    if (!CurrentCharacter)
    {
        return TEXT("Unknown");
    }
    
    return CurrentCharacter->GetCharacterName();
}

bool USkillTreeWidgetController::EquipSkillBySlot(const FName& SlotID)
{
    if (!SkillEquipmentComponent || !SkillTreeSubsystem || !CurrentCharacter)
    {
        return false;
    }
    
    // Obter CharacterID
    FName CharacterID = CurrentCharacter->GetCharacterUniqueID();
    
    // Buscar linha por CharacterID e SlotID
    FSkillTreeTableRow SkillRow = SkillTreeSubsystem->GetSkillTableRowBySlot(CharacterID, SlotID);
    
    if (SkillRow.SkillID.IsNone())
    {
        return false;
    }
    
    // Verificar se a habilidade está desbloqueada
    if (!SkillTreeSubsystem->SkillUnlocked(CurrentCharacter, SkillRow.SkillID))
    {
        return false;
    }
    
    // Equipar a habilidade
    // Aqui devemos usar uma InputTag adequada - você pode implementar conforme necessário
    FGameplayTag InputTag; // Por enquanto vazio, você pode implementar a lógica adequada
    
    return SkillEquipmentComponent->EquipSkill(SkillRow.SkillID, SlotID, InputTag);
}

bool USkillTreeWidgetController::UnequipSkillBySlot(const FName& SlotID)
{
    if (!SkillEquipmentComponent)
    {
        return false;
    }
    
    return SkillEquipmentComponent->UnequipSkill(SlotID);
}

void USkillTreeWidgetController::UnlockSkillsWithoutPrerequisites()
{
    if (!SkillTreeSubsystem || !CurrentCharacter)
    {
        return;
    }
    
    SkillTreeSubsystem->UnlockSkillsWithoutPrerequisites(CurrentCharacter);
}

TArray<FName> USkillTreeWidgetController::GetEquippedSkills() const
{
    if (!SkillEquipmentComponent)
    {
        return TArray<FName>();
    }
    
    TMap<FName, FName> AllEquipped = SkillEquipmentComponent->GetAllEquippedSkills();
    TArray<FName> EquippedSkills;
    
    for (const auto& Pair : AllEquipped)
    {
        if (!Pair.Value.IsNone())
        {
            EquippedSkills.Add(Pair.Value);
        }
    }
    
    return EquippedSkills;
}
