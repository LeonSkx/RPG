// Copyright Druid Mechanics

// Logs disabled for cleanup

#include "Progression/SkillTreeSubsystem.h"

#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"
#include "Character/RPGCharacter.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAbilitySystemLibrary.h"

#include "Interaction/PlayerInterface.h"
#include "Progression/ProgressionSubsystem.h"
#include "Progression/SkillTreeTableRow.h"
#include "Quest/QuestSubsystem.h"

// === CONFIGURAÇÃO ===



void USkillTreeSubsystem::SetSkillTreeDataTable(UDataTable* NewSkillTreeDataTable)
{
    SkillTreeDataTable = NewSkillTreeDataTable;
}

void USkillTreeSubsystem::SetCharacterSkillTable(const FName& CharacterID, UDataTable* DataTable)
{
    if (DataTable)
    {
        CharacterSkillTables.Add(CharacterID, DataTable);
    }
}

UDataTable* USkillTreeSubsystem::GetCharacterSkillTable(const FName& CharacterID) const
{
    if (const TObjectPtr<UDataTable>* FoundTable = CharacterSkillTables.Find(CharacterID))
    {
        return *FoundTable;
    }
    
    // Fallback para o Data Table global se não encontrar específico
    return SkillTreeDataTable;
}

FSkillTreeTableRow USkillTreeSubsystem::GetSkillTableRowBySlot(const FName& CharacterID, const FName& SlotID) const
{
    // Obter o Data Table específico do personagem
    UDataTable* TargetDataTable = GetCharacterSkillTable(CharacterID);
    
    if (!TargetDataTable)
    {
        return FSkillTreeTableRow();
    }
    
    // Buscar por SlotID (não precisa mais do CharacterID pois já está no Data Table correto)
    TArray<FName> RowNames = TargetDataTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        if (FSkillTreeTableRow* Row = TargetDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GetSkillTableRowBySlot")))
        {
            if (Row->SlotID == SlotID)
            {
                return *Row;
            }
        }
    }
    
    return FSkillTreeTableRow();
}

// === VERIFICAÇÕES ===

bool USkillTreeSubsystem::CanUnlockSkill(const ARPGCharacter* Character, const FName& SkillID) const
{
    if (!Character)
    {
        return false;
    }

    // Obter CharacterID e Data Table específico
    FName CharacterID = Character->GetCharacterUniqueID();
    UDataTable* TargetDataTable = GetCharacterSkillTable(CharacterID);
    
    if (!TargetDataTable)
    {
        return false;
    }

    // Verificar se a habilidade existe na árvore
    bool bSkillFound = false;
    TArray<FName> RowNames = TargetDataTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        if (FSkillTreeTableRow* Row = TargetDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("CanUnlockSkill")))
        {
            if (Row->SkillID == SkillID)
            {
                bSkillFound = true;
                break;
            }
        }
    }
    
    if (!bSkillFound)
    {
        return false;
    }

    // Verificar se já está desbloqueada
    if (SkillUnlocked(Character, SkillID))
    {
        return false;
    }

    // Buscar dados da habilidade no Data Table
    FSkillTreeTableRow SkillRow;
    bool bSkillDataFound = false;
    
    for (const FName& RowName : RowNames)
    {
        if (FSkillTreeTableRow* Row = TargetDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("CanUnlockSkill")))
        {
            if (Row->SkillID == SkillID)
            {
                SkillRow = *Row;
                bSkillDataFound = true;
                break;
            }
        }
    }
    
    if (!bSkillDataFound)
    {
        return false;
    }

    // Verificar nível necessário
    int32 XP = 0;
    if (Character->GetClass()->ImplementsInterface(UPlayerInterface::StaticClass()))
    {
        XP = IPlayerInterface::Execute_GetXP(Character);
    }
    else
    {
        // Character não implementa PlayerInterface
    }
    
    // Verificar se o ProgressionSubsystem está disponível
    if (const UGameInstance* GameInstance = Character->GetGameInstance())
    {
        if (const UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
        {
            int32 DirectXP = ProgressionSubsystem->GetCharacterXP(Character);
        }
    }
    
    int32 CharacterLevel = 1;
    if (Character->GetClass()->ImplementsInterface(UPlayerInterface::StaticClass()))
    {
        CharacterLevel = IPlayerInterface::Execute_FindLevelForXP(Character, XP);
    }
    else
    {
        // Character não implementa PlayerInterface para FindLevelForXP
    }
    if (CharacterLevel < SkillRow.RequiredLevel)
    {
        return false;
    }

    // Verificar pontos de magia
    int32 SpellPoints = 0;
    if (Character->GetClass()->ImplementsInterface(UPlayerInterface::StaticClass()))
    {
        SpellPoints = IPlayerInterface::Execute_GetSpellPoints(Character);
    }
    else
    {
        // Character não implementa PlayerInterface para GetSpellPoints
    }
    if (SpellPoints < SkillRow.RequiredSpellPoints)
    {
        return false;
    }

    // Verificar pré-requisitos
    if (!ArePrerequisitesMet(Character, SkillID))
    {
        return false;
    }

    // Verificar requisito de quest
    if (!CheckQuestRequirement(SkillRow.RequiredQuestID, const_cast<ARPGCharacter*>(Character)))
    {
        return false;
    }

    return true;
}

bool USkillTreeSubsystem::CanUnlockSkillBySlot(const ARPGCharacter* Character, const FName& SlotID) const
{
    if (!Character)
    {
        return false;
    }

    // Obter CharacterID do personagem
    FName CharacterID = Character->GetCharacterUniqueID();
    
    // Buscar linha por CharacterID e SlotID
    FSkillTreeTableRow SkillRow = GetSkillTableRowBySlot(CharacterID, SlotID);
    
    if (SkillRow.SkillID.IsNone())
    {
        return false;
    }

    // Usar a função original com o SkillID encontrado
    return CanUnlockSkill(Character, SkillRow.SkillID);
}

bool USkillTreeSubsystem::SkillUnlocked(const ARPGCharacter* Character, const FName& SkillID) const
{
    if (!Character)
    {
        return false;
    }

    FName CharacterID = Character->GetCharacterUniqueID();
    if (const TSet<FName>* UnlockedSkills = CharacterUnlockedSkills.Find(CharacterID))
    {
        return UnlockedSkills->Contains(SkillID);
    }

    return false;
}

bool USkillTreeSubsystem::SkillUnlockedBySlot(const ARPGCharacter* Character, const FName& SlotID) const
{
    if (!Character)
    {
        return false;
    }

    // Obter CharacterID do personagem
    FName CharacterID = Character->GetCharacterUniqueID();
    
    // Buscar linha por CharacterID e SlotID
    FSkillTreeTableRow SkillRow = GetSkillTableRowBySlot(CharacterID, SlotID);
    
    if (SkillRow.SkillID.IsNone())
    {
        return false;
    }

    // Usar a função original com o SkillID encontrado
    return SkillUnlocked(Character, SkillRow.SkillID);
}

bool USkillTreeSubsystem::SkillLockedBySlot(const ARPGCharacter* Character, const FName& SlotID) const
{
    // Simplesmente o inverso de SkillUnlockedBySlot
    bool bIsUnlocked = SkillUnlockedBySlot(Character, SlotID);
    bool bIsLocked = !bIsUnlocked;
    
    return bIsLocked;
}

bool USkillTreeSubsystem::ArePrerequisitesMet(const ARPGCharacter* Character, const FName& SkillID) const
{
    if (!Character)
    {
        return false;
    }

    // Obter CharacterID e Data Table específico
    FName CharacterID = Character->GetCharacterUniqueID();
    UDataTable* TargetDataTable = GetCharacterSkillTable(CharacterID);
    
    if (!TargetDataTable)
    {
        return false;
    }

    // Buscar dados da habilidade no Data Table
    FSkillTreeTableRow SkillRow;
    bool bSkillFound = false;
    TArray<FName> RowNames = TargetDataTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        if (FSkillTreeTableRow* Row = TargetDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("ArePrerequisitesMet")))
        {
            if (Row->SkillID == SkillID)
            {
                SkillRow = *Row;
                bSkillFound = true;
                break;
            }
        }
    }
    
    if (!bSkillFound)
    {
        return false;
    }

    // Verificar se todos os pré-requisitos estão desbloqueados
    for (const FName& Prerequisite : SkillRow.Prerequisites)
    {
        // Ignorar pré-requisitos "None" (erro de configuração)
        if (Prerequisite == NAME_None)
        {
            continue;
        }
        
        if (!SkillUnlocked(Character, Prerequisite))
        {
            return false;
        }
    }

    return true;
}

// === AÇÕES ===

bool USkillTreeSubsystem::UnlockSkill(ARPGCharacter* Character, const FName& SkillID)
{
    if (!CanUnlockSkill(Character, SkillID))
    {
        return false;
    }

    // Obter CharacterID e Data Table específico
    FName CharacterID = Character->GetCharacterUniqueID();
    UDataTable* TargetDataTable = GetCharacterSkillTable(CharacterID);
    
    if (!TargetDataTable)
    {
        return false;
    }

    // Buscar dados da habilidade no Data Table
    FSkillTreeTableRow SkillRow;
    bool bSkillFound = false;
    TArray<FName> RowNames = TargetDataTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        if (FSkillTreeTableRow* Row = TargetDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("UnlockSkill")))
        {
            if (Row->SkillID == SkillID)
            {
                SkillRow = *Row;
                bSkillFound = true;
                break;
            }
        }
    }
    
    if (!bSkillFound)
    {
        return false;
    }

    // Consumir pontos de magia
    if (Character->GetClass()->ImplementsInterface(UPlayerInterface::StaticClass()))
    {
        IPlayerInterface::Execute_AddToSpellPoints(Character, -SkillRow.RequiredSpellPoints);
    }
    else
    {
        // Character não implementa PlayerInterface para AddToSpellPoints
    }

    // Marcar como desbloqueada (NÃO conceder automaticamente)
    EnsureCharacterDataExists(Character);
    
    if (TSet<FName>* UnlockedSkills = CharacterUnlockedSkills.Find(CharacterID))
    {
        UnlockedSkills->Add(SkillID);
    }

    // NÃO conceder a habilidade automaticamente - apenas desbloquear
    // GrantSkillToCharacter(Character, SkillID); // REMOVIDO

    // Disparar evento de desbloqueio
    OnSkillUnlocked.Broadcast(Character, SkillID);
    
    // Notificar que pontos mudaram (para atualizar UI)
    NotifyPointsChanged();

    return true;
}

bool USkillTreeSubsystem::UnlockSkillBySlot(ARPGCharacter* Character, const FName& SlotID)
{
    if (!Character)
    {
        return false;
    }

    // Obter CharacterID do personagem
    FName CharacterID = Character->GetCharacterUniqueID();
    
    // Buscar linha por CharacterID e SlotID
    FSkillTreeTableRow SkillRow = GetSkillTableRowBySlot(CharacterID, SlotID);
    
    if (SkillRow.SkillID.IsNone())
    {
        return false;
    }

    // Usar a função original com o SkillID encontrado
    return UnlockSkill(Character, SkillRow.SkillID);
}

bool USkillTreeSubsystem::ForceUnlockSkill(ARPGCharacter* Character, const FName& SkillID)
{
    if (!Character)
    {
        return false;
    }
    
    // Obter CharacterID e Data Table específico
    FName CharacterID = Character->GetCharacterUniqueID();
    UDataTable* TargetDataTable = GetCharacterSkillTable(CharacterID);
    
    if (!TargetDataTable)
    {
        return false;
    }
    
    // Verificar se a habilidade existe no Data Table
    bool bSkillFound = false;
    TArray<FName> RowNames = TargetDataTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        if (FSkillTreeTableRow* Row = TargetDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("ForceUnlockSkill")))
        {
            if (Row->SkillID == SkillID)
            {
                bSkillFound = true;
                break;
            }
        }
    }
    
    if (!bSkillFound)
    {
        return false;
    }

    // Marcar como desbloqueada (sem verificar pré-requisitos)
    EnsureCharacterDataExists(Character);
    
    if (TSet<FName>* UnlockedSkills = CharacterUnlockedSkills.Find(CharacterID))
    {
        UnlockedSkills->Add(SkillID);
    }

    // Conceder a habilidade via GAS
    GrantSkillToCharacter(Character, SkillID);

    // Disparar evento
    OnSkillUnlocked.Broadcast(Character, SkillID);

    return true;
}

// === CONSULTAS ===

TArray<FName> USkillTreeSubsystem::GetUnlockedSkills(const ARPGCharacter* Character) const
{
    TArray<FName> UnlockedSkillsArray;

    if (!Character)
    {
        return UnlockedSkillsArray;
    }

    FName CharacterID = Character->GetCharacterUniqueID();
    if (const TSet<FName>* UnlockedSkills = CharacterUnlockedSkills.Find(CharacterID))
    {
        for (const FName& SkillID : *UnlockedSkills)
        {
            UnlockedSkillsArray.Add(SkillID);
        }
    }

    return UnlockedSkillsArray;
}

void USkillTreeSubsystem::NotifyPointsChanged()
{
    OnPointsChanged.Broadcast();
}

TArray<FName> USkillTreeSubsystem::GetAvailableSkills(const ARPGCharacter* Character) const
{
    TArray<FName> AvailableSkills;

    if (!Character)
    {
        return AvailableSkills;
    }

    // Obter CharacterID e Data Table específico
    FName CharacterID = Character->GetCharacterUniqueID();
    UDataTable* TargetDataTable = GetCharacterSkillTable(CharacterID);
    
    if (!TargetDataTable)
    {
        return AvailableSkills;
    }

    // Verificar todas as habilidades do Data Table
    TArray<FName> RowNames = TargetDataTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        if (FSkillTreeTableRow* Row = TargetDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GetAvailableSkills")))
        {
            const FName& SkillID = Row->SkillID;
            
            if (CanUnlockSkill(Character, SkillID))
            {
                AvailableSkills.Add(SkillID);
            }
        }
    }

    return AvailableSkills;
}

FSkillTreeNode USkillTreeSubsystem::GetSkillNode(const FName& SkillID) const
{
    // Buscar em todos os Data Tables configurados
    for (const auto& CharacterTable : CharacterSkillTables)
    {
        UDataTable* DataTable = CharacterTable.Value;
        if (!DataTable) continue;
        
        TArray<FName> RowNames = DataTable->GetRowNames();
        
        for (const FName& RowName : RowNames)
        {
            if (FSkillTreeTableRow* Row = DataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GetSkillNode")))
            {
                if (Row->SkillID == SkillID)
                {
                    // Converter FSkillTreeTableRow para FSkillTreeNode
                    FSkillTreeNode SkillNode;
                    SkillNode.CharacterID = Row->CharacterID;
                    SkillNode.SlotID = Row->SlotID;
                    SkillNode.SkillID = Row->SkillID;
                    SkillNode.AbilityClass = Row->AbilityClass;
                    SkillNode.RequiredLevel = Row->RequiredLevel;
                    SkillNode.RequiredSpellPoints = Row->RequiredSpellPoints;
                    SkillNode.Prerequisites = Row->Prerequisites;
                    SkillNode.SkillName = Row->SkillName;
                    SkillNode.SkillDescription = Row->SkillDescription;
                    SkillNode.SkillIcon = Row->SkillIcon;
                    // TODO: Adicionar RequiredQuestID ao FSkillTreeNode se necessário
                    return SkillNode;
                }
            }
        }
    }
    
    // Fallback para o Data Table global
    if (SkillTreeDataTable)
    {
        TArray<FName> RowNames = SkillTreeDataTable->GetRowNames();
        
        for (const FName& RowName : RowNames)
        {
            if (FSkillTreeTableRow* Row = SkillTreeDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GetSkillNode")))
            {
                if (Row->SkillID == SkillID)
                {
                    // Converter FSkillTreeTableRow para FSkillTreeNode
                    FSkillTreeNode SkillNode;
                    SkillNode.CharacterID = Row->CharacterID;
                    SkillNode.SlotID = Row->SlotID;
                    SkillNode.SkillID = SkillID;
                    SkillNode.AbilityClass = Row->AbilityClass;
                    SkillNode.RequiredLevel = Row->RequiredLevel;
                    SkillNode.RequiredSpellPoints = Row->RequiredSpellPoints;
                    SkillNode.Prerequisites = Row->Prerequisites;
                    SkillNode.SkillName = Row->SkillName;
                    SkillNode.SkillDescription = Row->SkillDescription;
                    SkillNode.SkillIcon = Row->SkillIcon;
                    // TODO: Adicionar RequiredQuestID ao FSkillTreeNode se necessário
                    return SkillNode;
                }
            }
        }
    }

    return FSkillTreeNode();
}

FSkillTreeNode USkillTreeSubsystem::GetSkillNodeBySlot(const FName& CharacterID, const FName& SlotID) const
{
    // Usar a nova função que retorna FSkillTreeTableRow
    FSkillTreeTableRow SkillRow = GetSkillTableRowBySlot(CharacterID, SlotID);
    
    if (SkillRow.SkillID.IsNone())
    {
        return FSkillTreeNode();
    }
    
    // Converter FSkillTreeTableRow para FSkillTreeNode
    FSkillTreeNode SkillNode;
    SkillNode.CharacterID = SkillRow.CharacterID;
    SkillNode.SlotID = SkillRow.SlotID;
    SkillNode.SkillID = SkillRow.SkillID;
    SkillNode.AbilityClass = SkillRow.AbilityClass;
    SkillNode.RequiredLevel = SkillRow.RequiredLevel;
    SkillNode.RequiredSpellPoints = SkillRow.RequiredSpellPoints;
    SkillNode.Prerequisites = SkillRow.Prerequisites;
    SkillNode.SkillName = SkillRow.SkillName;
    SkillNode.SkillDescription = SkillRow.SkillDescription;
    SkillNode.SkillIcon = SkillRow.SkillIcon;
    // TODO: Adicionar RequiredQuestID ao FSkillTreeNode se necessário
    
    return SkillNode;
}

int32 USkillTreeSubsystem::GetUnlockedSkillCount(const ARPGCharacter* Character) const
{
    if (!Character)
    {
        return 0;
    }

    FName CharacterID = Character->GetCharacterUniqueID();
    if (const TSet<FName>* UnlockedSkills = CharacterUnlockedSkills.Find(CharacterID))
    {
        return UnlockedSkills->Num();
    }

    return 0;
}

TArray<FName> USkillTreeSubsystem::GetAvailableSkillIDs() const
{
    TArray<FName> SkillIDs;
    
    // Buscar em todos os Data Tables configurados
    for (const auto& CharacterTable : CharacterSkillTables)
    {
        UDataTable* DataTable = CharacterTable.Value;
        if (!DataTable) continue;
        
        TArray<FName> RowNames = DataTable->GetRowNames();
        
        for (const FName& RowName : RowNames)
        {
            if (FSkillTreeTableRow* Row = DataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GetAvailableSkillIDs")))
            {
                SkillIDs.Add(Row->SkillID);
            }
        }
    }
    
    // Fallback para o Data Table global
    if (SkillTreeDataTable)
    {
        TArray<FName> RowNames = SkillTreeDataTable->GetRowNames();
        
        for (const FName& RowName : RowNames)
        {
            if (FSkillTreeTableRow* Row = SkillTreeDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GetAvailableSkillIDs")))
            {
                SkillIDs.Add(Row->SkillID);
            }
        }
    }
    
    return SkillIDs;
}

bool USkillTreeSubsystem::SkillTreeDataConnected() const
{
    // Verificar se há Data Tables específicos configurados
    if (CharacterSkillTables.Num() > 0)
    {
        return true;
    }
    
    // Fallback para o Data Table global
    return SkillTreeDataTable != nullptr;
}

// === VERIFICAÇÃO DE QUEST ===

bool USkillTreeSubsystem::CheckQuestRequirement(const FString& RequiredQuestID, ARPGCharacter* Character) const
{
    // Se não há quest requerida, sempre retorna true
    if (RequiredQuestID.IsEmpty())
    {
        return true;
    }
    
    // Se não há personagem, retorna false
    if (!Character)
    {
        return false;
    }
    
    // Obter o QuestSubsystem do GameInstance
    if (UGameInstance* GameInstance = Character->GetGameInstance())
    {
        if (UQuestSubsystem* QuestSubsystem = GameInstance->GetSubsystem<UQuestSubsystem>())
        {
            // Verificar se a quest foi completada
            // Nota: Assumindo que QuestSubsystem tem uma função IsQuestCompleted
            // Se não tiver, precisaremos implementar
            return QuestSubsystem->IsQuestCompleted(RequiredQuestID, Character);
        }
    }
    
    // Se não conseguir acessar o QuestSubsystem, retorna false por segurança
    return false;
}

FSkillTreeNode USkillTreeSubsystem::GetSkillData(const FName& CharacterID, const FName& SkillID) const
{
    if (UDataTable* CharacterTable = GetCharacterSkillTable(CharacterID))
    {
        TArray<FName> RowNames = CharacterTable->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            if (FSkillTreeTableRow* Row = CharacterTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GetSkillData")))
            {
                if (Row->SkillID == SkillID)
                {
                    FSkillTreeNode SkillNode;
                    SkillNode.CharacterID = CharacterID;
                    SkillNode.SkillID = SkillID;
                    SkillNode.SlotID = Row->SlotID;
                    SkillNode.AbilityClass = Row->AbilityClass;
                    SkillNode.RequiredLevel = Row->RequiredLevel;
                    SkillNode.RequiredSpellPoints = Row->RequiredSpellPoints;
                    SkillNode.Prerequisites = Row->Prerequisites;
                    SkillNode.SkillName = Row->SkillName;
                    SkillNode.SkillDescription = Row->SkillDescription;
                    SkillNode.SkillIcon = Row->SkillIcon;
                    return SkillNode;
                }
            }
        }
    }
    
    return FSkillTreeNode();
}







// === FUNÇÕES PRIVADAS ===

void USkillTreeSubsystem::EnsureCharacterDataExists(const ARPGCharacter* Character)
{
    if (Character)
    {
        FName CharacterID = Character->GetCharacterUniqueID();
        if (!CharacterUnlockedSkills.Contains(CharacterID))
        {
            CharacterUnlockedSkills.Add(CharacterID, TSet<FName>());
        }
        // REMOVIDO: CharacterEquippedSkills movido para SkillEquipmentComponent
    }
}

void USkillTreeSubsystem::GrantSkillToCharacter(ARPGCharacter* Character, const FName& SkillID)
{
    if (!Character)
    {
        return;
    }

    // Obter CharacterID e Data Table específico
    FName CharacterID = Character->GetCharacterUniqueID();
    UDataTable* TargetDataTable = GetCharacterSkillTable(CharacterID);
    
    if (!TargetDataTable)
    {
        return;
    }

    // Buscar dados da habilidade no Data Table
    FSkillTreeTableRow SkillRow;
    bool bSkillFound = false;
    TArray<FName> RowNames = TargetDataTable->GetRowNames();
    
    for (const FName& RowName : RowNames)
    {
        if (FSkillTreeTableRow* Row = TargetDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GrantSkillToCharacter")))
        {
            if (Row->SkillID == SkillID)
            {
                SkillRow = *Row;
                bSkillFound = true;
                break;
            }
        }
    }
    
    if (!bSkillFound)
    {
        return;
    }
    
    if (SkillRow.AbilityClass)
    {
        if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
        {
            // Criar spec da habilidade
            FGameplayAbilitySpec AbilitySpec(SkillRow.AbilityClass, 1);
            
            // Adicionar tag da habilidade se for RPGAbility
            if (const URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(AbilitySpec.Ability))
            {
                AbilitySpec.GetDynamicSpecSourceTags().AddTag(RPGAbility->StartupInputTag);
            }
            
            // Conceder a habilidade
            ASC->GiveAbility(AbilitySpec);
        }
    }
}

void USkillTreeSubsystem::UnlockSkillsWithoutPrerequisites(ARPGCharacter* Character)
{
    if (!Character)
    {
        return;
    }

    FName CharacterID = Character->GetCharacterUniqueID();
    UDataTable* TargetDataTable = GetCharacterSkillTable(CharacterID);
    
    if (!TargetDataTable)
    {
        return;
    }

    // Garantir que dados do personagem existam
    EnsureCharacterDataExists(Character);

    // Buscar todas as habilidades no Data Table
    TArray<FName> RowNames = TargetDataTable->GetRowNames();
    int32 UnlockedCount = 0;

    for (const FName& RowName : RowNames)
    {
        if (FSkillTreeTableRow* Row = TargetDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("UnlockSkillsWithoutPrerequisites")))
        {
            // Verificar se a habilidade não tem pré-requisitos
            if (Row->Prerequisites.Num() == 0)
            {
                // Verificar se já não está desbloqueada
                if (!SkillUnlocked(Character, Row->SkillID))
                {
                    // Desbloquear a habilidade
                    if (UnlockSkill(Character, Row->SkillID))
                    {
                        UnlockedCount++;
                    }
                }
            }
        }
    }
}

// === SISTEMA DE EQUIPAMENTO ===
// REMOVIDO: Funções de equipamento movidas para SkillEquipmentComponent
// O SkillTreeSubsystem agora foca apenas em desbloqueio e dados 
