#include "Quest/QuestSubsystem.h"
#include "Quest/QuestFunctionLibrary.h"
#include "Character/RPGCharacter.h"
#include "Engine/Engine.h"
#include "RPGGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Progression/ProgressionSubsystem.h"
#include "Inventory/Core/InventoryTypes.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Inventory/Core/InventorySubsystem.h"
#include "Party/PartySubsystem.h"
#include "Kismet/GameplayStatics.h"

UQuestSubsystem::UQuestSubsystem()
{
    QuestDataAssets.Empty();
}

// === INICIALIZAÇÃO ===

void UQuestSubsystem::SetQuestDataAssets(const TArray<UQuestDataAsset*>& InQuestDataAssets)
{
    QuestDataAssets = InQuestDataAssets;
    ClearQuestCache();
    BuildQuestCache();
}

void UQuestSubsystem::LoadQuestsFromDataAsset()
{
    if (QuestDataAssets.Num() == 0)
    {
        return;
    }
}

// === GESTÃO DE QUESTS ===

bool UQuestSubsystem::AcceptQuest(const FString& QuestID, ARPGCharacter* Character)
{
    if (!Character || QuestDataAssets.Num() == 0)
    {
        return false;
    }

    if (!bQuestCacheBuilt)
    {
        BuildQuestCache();
    }

    // Buscar dados da quest no cache
    const FQuestData* QuestDataPtr = FindQuestInCache(QuestID);
    if (!QuestDataPtr)
    {
        return false;
    }
    const FQuestData QuestData = *QuestDataPtr;
    
    if (QuestData.QuestID.IsEmpty())
    {
        return false;
    }

    // Verificar se pode aceitar
    if (!CanAcceptQuest(QuestData, Character))
    {
        return false;
    }

    // Verificar se já aceitou
    if (IsQuestActive(QuestID, Character))
    {
        return false;
    }

    // Verificar se já completou e se pode repetir
    if (IsQuestCompleted(QuestID, Character) && !QuestData.bCanBeRepeated)
    {
        return false;
    }

    // Criar progresso inicial
    FQuestProgress* Progress = GetOrCreateQuestProgress(QuestID, Character);
    if (!Progress)
    {
        return false;
    }

    // Configurar progresso inicial
    Progress->QuestID = QuestID;
    Progress->State = EQuestState::Active;
    Progress->bRewardsClaimed = false;

    // Copiar objetivos da quest
    Progress->ObjectiveProgress = QuestData.Objectives;
    for (FQuestObjective& Objective : Progress->ObjectiveProgress)
    {
        Objective.CurrentAmount = 0;
        Objective.bIsCompleted = false;
        // Disponibilidade inicial baseada nos requisitos
        Objective.bIsAvailable = Objective.RequiredObjectives.Num() == 0;
    }

    // ✅ NOVO: Verificar disponibilidade inicial dos objetivos
    UpdateObjectiveAvailability(Progress);

    // Disparar evento
    OnQuestAccepted.Broadcast(QuestID);

    return true;
}

bool UQuestSubsystem::CompleteQuest(const FString& QuestID, ARPGCharacter* Character)
{
    if (!Character || QuestDataAssets.Num() == 0)
    {
        return false;
    }

    // Verificar se quest está ativa
    if (!IsQuestActive(QuestID, Character))
    {
        return false;
    }

    // Verificar se pode completar
    FQuestProgress* Progress = GetOrCreateQuestProgress(QuestID, Character);
    if (!Progress || !CanCompleteQuest(*Progress))
    {
        return false;
    }

    // Buscar dados da quest para obter recompensas
    if (!bQuestCacheBuilt)
    {
        BuildQuestCache();
    }
    FQuestData QuestData;
    if (const FQuestData* QuestDataPtr = FindQuestInCache(QuestID))
    {
        QuestData = *QuestDataPtr;
    }

    // Marcar como completada
    Progress->State = EQuestState::Completed;

    // Distribuir recompensas
    if (!QuestData.QuestID.IsEmpty())
    {
        // XP da quest
        if (QuestData.Rewards.Experience > 0)
        {
            if (QuestData.Rewards.bShareRewardsWithGroup)
            {
                // Distribuir XP para todo o grupo via GAS
                if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
                {
                    if (UProgressionSubsystem* ProgressionSystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
                    {
                        ProgressionSystem->AddGroupXPViaGAS(QuestData.Rewards.Experience);
                    }
                }
            }
            else
            {
                // Distribuir XP só para quem completou
                if (Character->Implements<UPlayerInterface>())
                {
                    // Usar o mesmo sistema do inimigo - via GAS IncomingXP
                    const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
                    FGameplayEventData Payload;
                    Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;
                    Payload.EventMagnitude = QuestData.Rewards.Experience;

                    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
                        Character, 
                        GameplayTags.Attributes_Meta_IncomingXP, 
                        Payload
                    );
                }
            }
        }

        // Gold da quest
        if (QuestData.Rewards.Gold > 0)
        {
            // Adicionar gold ao InventorySubsystem
            if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
            {
                if (UInventorySubsystem* Inventory = GameInstance->GetSubsystem<UInventorySubsystem>())
                {
                    Inventory->AddGold(QuestData.Rewards.Gold);
                }
            }
        }

        // AttributePoints da quest
        if (QuestData.Rewards.AttributePoints > 0)
        {
            if (QuestData.Rewards.bShareRewardsWithGroup)
            {
                // Distribuir pontos para todo o grupo
                if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
                {
                    if (UProgressionSubsystem* ProgressionSystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
                    {
                        ProgressionSystem->AddGroupAttributePoints(QuestData.Rewards.AttributePoints);
                    }
                }
            }
            else
            {
                // Distribuir pontos só para quem completou
                if (Character->Implements<UPlayerInterface>())
                {
                    IPlayerInterface::Execute_AddToAttributePoints(Character, QuestData.Rewards.AttributePoints);
                }
            }
        }

        // SpellPoints da quest
        if (QuestData.Rewards.SpellPoints > 0)
        {
            if (QuestData.Rewards.bShareRewardsWithGroup)
            {
                // Distribuir pontos para todo o grupo
                if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
                {
                    if (UProgressionSubsystem* ProgressionSystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
                    {
                        ProgressionSystem->AddGroupSpellPoints(QuestData.Rewards.SpellPoints);
                    }
                }
            }
            else
            {
                // Distribuir pontos só para quem completou
                if (Character->Implements<UPlayerInterface>())
                {
                    IPlayerInterface::Execute_AddToSpellPoints(Character, QuestData.Rewards.SpellPoints);
                }
            }
        }

        // Marcar recompensas como coletadas
        Progress->bRewardsClaimed = true;
    }

    // Disparar evento
    OnQuestCompleted.Broadcast(QuestID);

    return true;
}

bool UQuestSubsystem::AbandonQuest(const FString& QuestID, ARPGCharacter* Character)
{
    if (!Character)
    {
        return false;
    }

    // Verificar se quest está ativa
    if (!IsQuestActive(QuestID, Character))
    {
        return false;
    }

    // Respeitar configuração de abandonável
    if (!bQuestCacheBuilt)
    {
        BuildQuestCache();
    }
    if (const FQuestData* QuestDataPtr = FindQuestInCache(QuestID))
    {
        if (!QuestDataPtr->bCanBeAbandoned)
        {
            return false;
        }
    }

    // Marcar como abandonada
    UpdateQuestState(QuestID, EQuestState::Abandoned, Character);

    return true;
}

bool UQuestSubsystem::FailQuest(const FString& QuestID, ARPGCharacter* Character)
{
    if (!Character)
    {
        return false;
    }

    // Verificar se quest está ativa
    if (!IsQuestActive(QuestID, Character))
    {
        return false;
    }

    // Marcar como falhada
    UpdateQuestState(QuestID, EQuestState::Failed, Character);

    // Disparar evento
    OnQuestFailed.Broadcast(QuestID);

    return true;
}

// === ATUALIZAÇÃO DE PROGRESSO ===

void UQuestSubsystem::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress, ARPGCharacter* Character)
{
    if (!Character)
    {
        return;
    }

    FQuestProgress* QuestProgress = GetOrCreateQuestProgress(QuestID, Character);
    if (!QuestProgress || QuestProgress->State != EQuestState::Active)
    {
        return;
    }

    // Encontrar e atualizar objetivo
    for (FQuestObjective& Objective : QuestProgress->ObjectiveProgress)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            // ✅ NOVO: Verificar se objetivo está disponível
            if (!Objective.bIsAvailable)
            {
                return; // Objetivo não está disponível ainda
            }

            const int32 PreviousAmount = Objective.CurrentAmount;
            const bool bPrevCompleted = Objective.bIsCompleted;
            Objective.CurrentAmount = FMath::Clamp(Progress, 0, Objective.RequiredAmount);
            Objective.bIsCompleted = IsObjectiveCompleted(Objective);

            // ✅ NOVO: Atualizar disponibilidade de outros objetivos
            UpdateObjectiveAvailability(QuestProgress);

            // Disparar evento apenas se houve mudança
            if (Objective.CurrentAmount != PreviousAmount || Objective.bIsCompleted != bPrevCompleted)
            {
                OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID, Objective.CurrentAmount);
            }

            // Verificar se quest foi completada
            if (CanCompleteQuest(*QuestProgress))
            {
                CompleteQuest(QuestID, Character);
            }

            break;
        }
    }
}

void UQuestSubsystem::UpdateKillProgress(const FString& QuestID, const FString& EnemyType, int32 Amount, ARPGCharacter* Character)
{
    if (!Character)
    {
        return;
    }

    if (EnemyType.IsEmpty() || Amount == 0)
    {
        return;
    }

    FQuestProgress* QuestProgress = GetOrCreateQuestProgress(QuestID, Character);
    if (!QuestProgress || QuestProgress->State != EQuestState::Active)
    {
        return;
    }

    // Encontrar objetivos do tipo Kill
    for (FQuestObjective& Objective : QuestProgress->ObjectiveProgress)
    {
        if (Objective.ObjectiveType == EObjectiveType::Kill && Objective.TargetID == EnemyType)
        {
            // ✅ NOVO: Verificar se objetivo está disponível
            if (!Objective.bIsAvailable)
            {
                continue; // Pular objetivo não disponível
            }

            const int32 PreviousAmount = Objective.CurrentAmount;
            const bool bPrevCompleted = Objective.bIsCompleted;
            Objective.CurrentAmount = FMath::Clamp(Objective.CurrentAmount + Amount, 0, Objective.RequiredAmount);
            Objective.bIsCompleted = IsObjectiveCompleted(Objective);

            // ✅ NOVO: Atualizar disponibilidade de outros objetivos
            UpdateObjectiveAvailability(QuestProgress);

            // Disparar evento apenas se houve mudança
            if (Objective.CurrentAmount != PreviousAmount || Objective.bIsCompleted != bPrevCompleted)
            {
                OnObjectiveUpdated.Broadcast(QuestID, Objective.ObjectiveID, Objective.CurrentAmount);
            }

            // Verificar se quest foi completada
            if (CanCompleteQuest(*QuestProgress))
            {
                CompleteQuest(QuestID, Character);
            }
        }
    }
}

void UQuestSubsystem::UpdateCollectProgress(const FString& QuestID, const FString& ItemID, int32 Amount, ARPGCharacter* Character)
{
    if (!Character)
    {
        return;
    }

    if (ItemID.IsEmpty() || Amount == 0)
    {
        return;
    }

    FQuestProgress* QuestProgress = GetOrCreateQuestProgress(QuestID, Character);
    if (!QuestProgress || QuestProgress->State != EQuestState::Active)
    {
        return;
    }

    // Encontrar objetivos do tipo Collect
    for (FQuestObjective& Objective : QuestProgress->ObjectiveProgress)
    {
        if (Objective.ObjectiveType == EObjectiveType::Collect && Objective.TargetID == ItemID)
        {
            // ✅ NOVO: Verificar se objetivo está disponível
            if (!Objective.bIsAvailable)
            {
                continue; // Pular objetivo não disponível
            }

            const int32 PreviousAmount = Objective.CurrentAmount;
            const bool bPrevCompleted = Objective.bIsCompleted;
            Objective.CurrentAmount = FMath::Clamp(Objective.CurrentAmount + Amount, 0, Objective.RequiredAmount);
            Objective.bIsCompleted = IsObjectiveCompleted(Objective);

            // ✅ NOVO: Atualizar disponibilidade de outros objetivos
            UpdateObjectiveAvailability(QuestProgress);

            // Disparar evento apenas se houve mudança
            if (Objective.CurrentAmount != PreviousAmount || Objective.bIsCompleted != bPrevCompleted)
            {
                OnObjectiveUpdated.Broadcast(QuestID, Objective.ObjectiveID, Objective.CurrentAmount);
            }

            // Verificar se quest foi completada
            bool bCanComplete = CanCompleteQuest(*QuestProgress);
            
            if (bCanComplete)
            {
                CompleteQuest(QuestID, Character);
            }
        }
    }
}

void UQuestSubsystem::UpdateKillProgressAuto(const FString& EnemyType, int32 Amount, ARPGCharacter* Character)
{
    if (!Character)
    {
        return;
    }

    if (EnemyType.IsEmpty() || Amount == 0)
    {
        return;
    }

    // Buscar TODAS as quests ativas do personagem
    TArray<FQuestProgress> ActiveQuests = GetActiveQuests(Character);
    
    for (const FQuestProgress& Quest : ActiveQuests)
    {
        bool bQuestHasMatchingObjective = false;
        // Verificar se esta quest tem pelo menos um objetivo de matar este inimigo
        for (const FQuestObjective& Objective : Quest.ObjectiveProgress)
        {
            if (Objective.ObjectiveType == EObjectiveType::Kill && Objective.TargetID == EnemyType)
            {
                bQuestHasMatchingObjective = true;
                break; // basta um; a função interna atualiza todos os objetivos correspondentes
            }
        }
        if (bQuestHasMatchingObjective)
        {
            UpdateKillProgress(Quest.QuestID, EnemyType, Amount, Character);
        }
    }
}

void UQuestSubsystem::UpdateCollectProgressAuto(const FString& ItemID, int32 Amount, ARPGCharacter* Character)
{
    if (!Character)
    {
        return;
    }

    if (ItemID.IsEmpty() || Amount == 0)
    {
        return;
    }

    // Buscar TODAS as quests ativas do personagem
    TArray<FQuestProgress> ActiveQuests = GetActiveQuests(Character);
    
    for (const FQuestProgress& Quest : ActiveQuests)
    {
        bool bQuestHasMatchingObjective = false;
        // Verificar se esta quest tem pelo menos um objetivo de coletar este item
        for (const FQuestObjective& Objective : Quest.ObjectiveProgress)
        {
            if (Objective.ObjectiveType == EObjectiveType::Collect && Objective.TargetID == ItemID)
            {
                bQuestHasMatchingObjective = true;
                break; // basta um; a função interna atualiza todos os objetivos correspondentes
            }
        }
        if (bQuestHasMatchingObjective)
        {
            UpdateCollectProgress(Quest.QuestID, ItemID, Amount, Character);
        }
    }
}

// === CONSULTA DE DADOS ===

bool UQuestSubsystem::IsQuestActive(const FString& QuestID, ARPGCharacter* Character) const
{
    if (!Character)
    {
        return false;
    }

    const FQuestProgress* Progress = GetQuestProgressInternal(QuestID, Character);
    return Progress && Progress->State == EQuestState::Active;
}

bool UQuestSubsystem::IsQuestCompleted(const FString& QuestID, ARPGCharacter* Character) const
{
    if (!Character)
    {
        return false;
    }

    const FQuestProgress* Progress = GetQuestProgressInternal(QuestID, Character);
    return Progress && Progress->State == EQuestState::Completed;
}

FQuestProgress UQuestSubsystem::GetQuestProgress(const FString& QuestID, ARPGCharacter* Character) const
{
	if (!Character)
	{
		return FQuestProgress();
	}

	const FQuestProgress* Progress = GetQuestProgressInternal(QuestID, Character);
	if (!Progress)
	{
		return FQuestProgress();
	}
	
	return *Progress;
}

TArray<FQuestProgress> UQuestSubsystem::GetActiveQuests(ARPGCharacter* Character) const
{
    TArray<FQuestProgress> Result;
    
    if (!Character)
    {
        return Result;
    }

    // ✅ USAR CharacterUniqueID para consistência!
    FName CharacterID = Character->GetCharacterUniqueID();
    const FString CharacterKeyPrefix = CharacterID.ToString() + TEXT("_");
    
    // Reserva aproximada (micro-otimização): assume até 16 por personagem
    Result.Reserve(16);

    for (const auto& Pair : QuestProgressMap)
    {
        if (Pair.Key.StartsWith(CharacterKeyPrefix) && Pair.Value.State == EQuestState::Active)
        {
            Result.Add(Pair.Value);
        }
    }

    return Result;
}

FText UQuestSubsystem::GetQuestNameByID(const FString& QuestID) const
{
    if (QuestID.IsEmpty())
    {
        return FText::GetEmpty();
    }

    if (!bQuestCacheBuilt)
    {
        const_cast<UQuestSubsystem*>(this)->BuildQuestCache();
    }

    const FQuestData* Data = FindQuestInCache(QuestID);
    if (Data)
    {
        return Data->QuestName;
    }
    
    return FText::GetEmpty();
}

FText UQuestSubsystem::GetQuestTypeTextByID(const FString& QuestID) const
{
    if (QuestID.IsEmpty())
    {
        return FText::GetEmpty();
    }

    if (!bQuestCacheBuilt)
    {
        const_cast<UQuestSubsystem*>(this)->BuildQuestCache();
    }

    const FQuestData* Data = FindQuestInCache(QuestID);
    if (Data)
    {
        return UQuestFunctionLibrary::GetQuestTypeText(Data->QuestType);
    }

    return FText::GetEmpty();
}

TArray<FQuestProgress> UQuestSubsystem::GetCompletedQuests(ARPGCharacter* Character) const
{
    TArray<FQuestProgress> Result;
    
    if (!Character)
    {
        return Result;
    }

    // ✅ USAR CharacterUniqueID para consistência!
    FName CharacterID = Character->GetCharacterUniqueID();
    const FString CharacterKeyPrefix = CharacterID.ToString() + TEXT("_");
    
    // Reserva aproximada (micro-otimização): assume até 16 por personagem
    Result.Reserve(16);

    for (const auto& Pair : QuestProgressMap)
    {
        if (Pair.Key.StartsWith(CharacterKeyPrefix) && Pair.Value.State == EQuestState::Completed)
        {
            Result.Add(Pair.Value);
        }
    }

    return Result;
}

TArray<FQuestData> UQuestSubsystem::GetAvailableQuests(ARPGCharacter* Character) const
{
    TArray<FQuestData> Result;
    if (!Character || QuestDataAssets.Num() == 0)
    {
        return Result;
    }

    // Garantir cache
    if (!bQuestCacheBuilt)
    {
        const_cast<UQuestSubsystem*>(this)->BuildQuestCache();
    }

    // Filtrar por requisitos e estado atual
    // Reserva aproximada (micro-otimização)
    Result.Reserve(32);

    for (const auto& Pair : QuestDataCache)
    {
        const FQuestData& QuestData = Pair.Value;
        const bool bNotActive = !IsQuestActive(QuestData.QuestID, Character);
        const bool bNotCompletedOrRepeatable = !IsQuestCompleted(QuestData.QuestID, Character) || QuestData.bCanBeRepeated;
        if (bNotActive && bNotCompletedOrRepeatable && CanAcceptQuest(QuestData, Character))
        {
            Result.Add(QuestData);
        }
    }

    return Result;
}

// ✅ NOVAS FUNÇÕES: Verificação de objetivos

bool UQuestSubsystem::IsObjectiveAvailable(const FString& QuestID, const FString& ObjectiveID, ARPGCharacter* Character) const
{
    if (!Character)
    {
        return false;
    }

    const FQuestProgress* Progress = GetQuestProgressInternal(QuestID, Character);
    if (!Progress)
    {
        return false;
    }

    for (const FQuestObjective& Objective : Progress->ObjectiveProgress)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.bIsAvailable;
        }
    }

    return false;
}

bool UQuestSubsystem::IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID, ARPGCharacter* Character) const
{
    if (!Character)
    {
        return false;
    }

    const FQuestProgress* Progress = GetQuestProgressInternal(QuestID, Character);
    if (!Progress)
    {
        return false;
    }

    for (const FQuestObjective& Objective : Progress->ObjectiveProgress)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.bIsCompleted;
        }
    }

    return false;
}

int32 UQuestSubsystem::GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, ARPGCharacter* Character) const
{
    if (!Character)
    {
        return 0;
    }

    const FQuestProgress* Progress = GetQuestProgressInternal(QuestID, Character);
    if (!Progress)
    {
        return 0;
    }

    for (const FQuestObjective& Objective : Progress->ObjectiveProgress)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.CurrentAmount;
        }
    }

    return 0;
}

// === FUNÇÕES DE DEBUG ===

void UQuestSubsystem::DebugQuestInfo(const FString& QuestID, ARPGCharacter* Character)
{
    if (!Character || QuestDataAssets.Num() == 0)
    {
        return;
    }

    FQuestData QuestData;
    for (UQuestDataAsset* DA : QuestDataAssets)
    {
        if (DA)
        {
            QuestData = DA->FindQuest(QuestID);
            if (!QuestData.QuestID.IsEmpty())
            {
                break;
            }
        }
    }
    
    if (QuestData.QuestID.IsEmpty())
    {
        return;
    }

    FQuestProgress Progress = GetQuestProgress(QuestID, Character);
}

void UQuestSubsystem::DebugShowAllActiveQuests(ARPGCharacter* Character)
{
    if (!Character)
    {
        return;
    }

    TArray<FQuestProgress> ActiveQuests = GetActiveQuests(Character);
}

void UQuestSubsystem::DebugAcceptQuest(const FString& QuestID, ARPGCharacter* Character)
{
    AcceptQuest(QuestID, Character);
}

void UQuestSubsystem::DebugQuestDataAsset()
{
    // Função de debug vazia - logs removidos
}

// === FUNÇÕES INTERNAS ===

bool UQuestSubsystem::CanAcceptQuest(const FQuestData& QuestData, ARPGCharacter* Character) const
{
    if (!Character)
    {
        return false;
    }

    // Verificar requisitos de nível
    if (!CheckLevelRequirement(QuestData.Prerequisites, Character))
    {
        return false;
    }

    // Verificar quests pré-requisitas
    if (!CheckQuestRequirements(QuestData.Prerequisites.RequiredQuests, Character))
    {
        return false;
    }

    // Verificar itens pré-requisitos
    if (!CheckItemRequirements(QuestData.Prerequisites.RequiredItems, Character))
    {
        return false;
    }

    return true;
}

// === CACHE DE QUESTS ===

void UQuestSubsystem::BuildQuestCache()
{
    QuestDataCache.Empty();
    for (UQuestDataAsset* DA : QuestDataAssets)
    {
        if (!DA)
        {
            continue;
        }

        // Atualmente populamos a partir de Main Story
        const TArray<FQuestData>& MainQuests = DA->MainStoryQuests;
        for (const FQuestData& QuestData : MainQuests)
        {
            if (!QuestData.QuestID.IsEmpty())
            {
                QuestDataCache.FindOrAdd(QuestData.QuestID, QuestData);
            }
        }
    }
    bQuestCacheBuilt = true;
}

const FQuestData* UQuestSubsystem::FindQuestInCache(const FString& QuestID) const
{
    return QuestDataCache.Find(QuestID);
}

void UQuestSubsystem::ClearQuestCache()
{
    QuestDataCache.Empty();
    bQuestCacheBuilt = false;
}

bool UQuestSubsystem::CanCompleteQuest(const FQuestProgress& Progress) const
{
    if (Progress.State != EQuestState::Active)
    {
        return false;
    }

    // Verificar se todos os objetivos obrigatórios foram completados
    for (const FQuestObjective& Objective : Progress.ObjectiveProgress)
    {
        // ✅ NOVO: Considerar apenas objetivos disponíveis
        if (!Objective.bIsOptional && Objective.bIsAvailable && !Objective.bIsCompleted)
        {
            return false;
        }
    }

    return true;
}

void UQuestSubsystem::UpdateQuestState(const FString& QuestID, EQuestState NewState, ARPGCharacter* Character)
{
    if (!Character)
    {
        return;
    }

    FQuestProgress* Progress = GetOrCreateQuestProgress(QuestID, Character);
    if (Progress)
    {
        Progress->State = NewState;
    }
}

bool UQuestSubsystem::IsObjectiveCompleted(const FQuestObjective& Objective) const
{
    return Objective.CurrentAmount >= Objective.RequiredAmount;
}

bool UQuestSubsystem::CheckLevelRequirement(const FQuestPrerequisites& Prerequisites, ARPGCharacter* Character) const
{
    if (!Character)
    {
        return false;
    }

    int32 RequiredLevel = Prerequisites.RequiredLevel;
    
    if (Prerequisites.bCheckPartyAverageLevel)
    {
        // ✅ Verificar média da party
        return CheckPartyAverageLevel(RequiredLevel, Character);
    }
    else
    {
        // ✅ Verificar apenas o personagem específico
        return CheckCharacterLevel(RequiredLevel, Character);
    }
}

bool UQuestSubsystem::CheckPartyAverageLevel(int32 RequiredLevel, ARPGCharacter* Character) const
{
    if (!Character)
    {
        return false;
    }

    // Obter PartySubsystem
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
        {
            TArray<ARPGCharacter*> PartyMembers = PartySubsystem->GetPartyMembers();
            
            if (PartyMembers.Num() == 0)
            {
                return false;
            }

            // Calcular média dos níveis
            int32 TotalLevel = 0;
            int32 ValidMembers = 0;
            
            for (ARPGCharacter* Member : PartyMembers)
            {
                if (Member && Member->Implements<UCombatInterface>())
                {
                    int32 MemberLevel = ICombatInterface::Execute_GetCharacterLevel(Member);
                    TotalLevel += MemberLevel;
                    ValidMembers++;
                }
            }

            if (ValidMembers == 0)
            {
                return false;
            }

            float AverageLevel = (float)TotalLevel / ValidMembers;

            return AverageLevel >= RequiredLevel;
        }
    }

    return false;
}

bool UQuestSubsystem::CheckCharacterLevel(int32 RequiredLevel, ARPGCharacter* Character) const
{
    if (!Character || !Character->Implements<UCombatInterface>())
    {
        return false;
    }

    int32 CharacterLevel = ICombatInterface::Execute_GetCharacterLevel(Character);

    return CharacterLevel >= RequiredLevel;
}

bool UQuestSubsystem::CheckQuestRequirements(const TArray<FString>& RequiredQuests, ARPGCharacter* Character) const
{
    if (!Character || RequiredQuests.Num() == 0)
    {
        return true; // Se não há quests requeridas, sempre permite
    }

    // Verificar cada quest pré-requisita
    for (const FString& RequiredQuestID : RequiredQuests)
    {
        if (!IsQuestCompleted(RequiredQuestID, Character))
        {
            return false;
        }
    }

    return true;
}

bool UQuestSubsystem::CheckItemRequirements(const TArray<FString>& RequiredItems, ARPGCharacter* Character) const
{
	if (!Character || RequiredItems.Num() == 0)
	{
		return true; // Se não há itens requeridos, sempre permite
	}

	// Inventário pessoal removido: verificar somente no inventário compartilhado
	UInventorySubsystem* SharedInv = Character->GetGameInstance()->GetSubsystem<UInventorySubsystem>();
	UInventorySubsystem* Inv = SharedInv;
	if (!Inv)
	{
		return false;
	}

	for (const FString& RequiredItemID : RequiredItems)
	{
		bool bFoundItem = false;
		// Procurar pelo ID entre todos os itens do inventário
		TArray<FInventoryItem> All = Inv->GetAllItems();
		for (const FInventoryItem& Item : All)
		{
			if (Item.ItemData && Item.ItemData->ItemID == RequiredItemID)
			{
				bFoundItem = true;
				break;
			}
		}
		if (!bFoundItem) return false;
	}
	return true;
}

FQuestProgress* UQuestSubsystem::GetOrCreateQuestProgress(const FString& QuestID, ARPGCharacter* Character)
{
    if (!Character)
    {
        return nullptr;
    }

    // ✅ USAR GenerateQuestKey para consistência!
    FString Key = GenerateQuestKey(Character, QuestID);
    
    // Procurar progresso existente
    if (FQuestProgress* ExistingProgress = QuestProgressMap.Find(Key))
    {
        return ExistingProgress;
    }

    // Criar novo progresso
    FQuestProgress NewProgress;
    NewProgress.QuestID = QuestID;
    NewProgress.State = EQuestState::NotStarted;
    NewProgress.bRewardsClaimed = false;

    QuestProgressMap.Add(Key, NewProgress);
    return QuestProgressMap.Find(Key);
}

const FQuestProgress* UQuestSubsystem::GetQuestProgressInternal(const FString& QuestID, ARPGCharacter* Character) const
{
    if (!Character)
    {
        return nullptr;
    }

    // ✅ USAR GenerateQuestKey para consistência!
    FString Key = GenerateQuestKey(Character, QuestID);
    return QuestProgressMap.Find(Key);
}

// === FUNÇÕES INTERNAS - KEY GENERATION ===

FString UQuestSubsystem::GenerateQuestKey(ARPGCharacter* Character, const FString& QuestID) const
{
    if (!Character)
    {
        return FString();
    }
    
    // ✅ USAR O SISTEMA EXISTENTE DE CHARACTERUNIQUEID!
    FName CharacterID = Character->GetCharacterUniqueID();
    return FString::Printf(TEXT("%s_%s"), *CharacterID.ToString(), *QuestID);
}

// === FUNÇÕES INTERNAS - VALIDAÇÕES ===

// === ESCUTA DE EVENTOS ===

void UQuestSubsystem::OnInventoryItemChanged(EItemCategory Category, const FInventoryItem& Item)
{
    if (!Item.ItemData)
    {
        return;
    }

    if (Item.Quantity <= 0)
    {
        return;
    }

    // Verificar se o item tem um ID válido
    if (Item.ItemData->ItemID.IsEmpty() || Item.ItemData->ItemID == "None_Item")
    {
        return;
    }

    // ✅ VALIDAÇÃO DE CATEGORIA (atual): processar apenas itens da categoria Quest
    if (Category != EItemCategory::Valuable)
    {
        return;
    }

    // Usar apenas o ItemID do ItemDataAsset (já deve estar no formato correto)
    FString ItemID = Item.ItemData->ItemID;

    // ✅ VALIDAÇÃO DE ID: Processar apenas itens da categoria Quest (IDs livres)
    // O usuário pode definir qualquer ID para seus itens de quest

    // Early-out: verifique se há algum objetivo de coleta relevante ativo
    bool bHasRelevantObjective = false;
    for (const auto& Pair : QuestProgressMap)
    {
        const FQuestProgress& Progress = Pair.Value;
        if (Progress.State != EQuestState::Active)
        {
            continue;
        }
        for (const FQuestObjective& Obj : Progress.ObjectiveProgress)
        {
            if (Obj.ObjectiveType == EObjectiveType::Collect && Obj.TargetID == ItemID && Obj.bIsAvailable && !Obj.bIsCompleted)
            {
                bHasRelevantObjective = true;
                break;
            }
        }
        if (bHasRelevantObjective)
        {
            break;
        }
    }
    if (!bHasRelevantObjective)
    {
        return;
    }

    // Buscar todos os personagens no mundo para atualizar suas quests
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        TSubclassOf<ARPGCharacter> CharacterClass = ARPGCharacter::StaticClass();
        UGameplayStatics::GetAllActorsOfClass(World, CharacterClass, FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (ARPGCharacter* Character = Cast<ARPGCharacter>(Actor))
            {
                if (Character->Implements<UPlayerInterface>())
                {
                    // Atualizar progresso de coleta automaticamente
                    UpdateCollectProgressAuto(ItemID, Item.Quantity, Character);
                }
            }
        }
    }


}

// ✅ NOVA FUNÇÃO: Atualizar disponibilidade dos objetivos
void UQuestSubsystem::UpdateObjectiveAvailability(FQuestProgress* QuestProgress)
{
    if (!QuestProgress)
    {
        return;
    }

    // Recalcular disponibilidade de todos os objetivos
    for (FQuestObjective& Objective : QuestProgress->ObjectiveProgress)
    {
        // Objetivos já completados permanecem disponíveis
        if (Objective.bIsCompleted)
        {
            Objective.bIsAvailable = true;
            continue;
        }

        // Sem requisitos: sempre disponível
        if (Objective.RequiredObjectives.Num() == 0)
        {
            Objective.bIsAvailable = true;
            continue;
        }

        // Verificar se todos os objetivos requeridos foram completados
        bool bAllRequirementsMet = true;
        for (const FString& RequiredObjectiveID : Objective.RequiredObjectives)
        {
            bool bFoundRequired = false;
            bool bRequiredCompleted = false;

            for (const FQuestObjective& RequiredObjective : QuestProgress->ObjectiveProgress)
            {
                if (RequiredObjective.ObjectiveID == RequiredObjectiveID)
                {
                    bFoundRequired = true;
                    bRequiredCompleted = RequiredObjective.bIsCompleted;
                    break;
                }
            }

            if (!bFoundRequired || !bRequiredCompleted)
            {
                bAllRequirementsMet = false;
                break;
            }
        }

        Objective.bIsAvailable = bAllRequirementsMet;
    }
}