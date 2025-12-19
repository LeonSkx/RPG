#include "UI/Quest/QuestWidget.h"
#include "Components/TextBlock.h"
#include "Quest/QuestSubsystem.h"
#include "Party/PartySubsystem.h"
#include "Character/RPGCharacter.h"

void UQuestWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        QuestSubsystem = GameInstance->GetSubsystem<UQuestSubsystem>();
        PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>();
    }

    if (QuestSubsystem)
    {
        QuestSubsystem->OnQuestAccepted.AddDynamic(this, &UQuestWidget::HandleQuestAccepted);
        QuestSubsystem->OnQuestCompleted.AddDynamic(this, &UQuestWidget::HandleQuestCompleted);
        QuestSubsystem->OnQuestFailed.AddDynamic(this, &UQuestWidget::HandleQuestFailed);
    }

    UpdateActiveQuestCount();
}

void UQuestWidget::NativeDestruct()
{
    if (QuestSubsystem)
    {
        QuestSubsystem->OnQuestAccepted.RemoveDynamic(this, &UQuestWidget::HandleQuestAccepted);
        QuestSubsystem->OnQuestCompleted.RemoveDynamic(this, &UQuestWidget::HandleQuestCompleted);
        QuestSubsystem->OnQuestFailed.RemoveDynamic(this, &UQuestWidget::HandleQuestFailed);
    }

    Super::NativeDestruct();
}

void UQuestWidget::UpdateActiveQuestCount()
{
    if ((!ActiveQuestNameText && !ActiveQuestTypeText) || !PartySubsystem || !QuestSubsystem)
    {
        return;
    }

    ARPGCharacter* ActiveCharacter = PartySubsystem->GetActivePartyMember();
    if (!ActiveCharacter)
    {
        if (ActiveQuestNameText)
        {
            ActiveQuestNameText->SetText(FText::GetEmpty());
        }
        if (ActiveQuestTypeText)
        {
            ActiveQuestTypeText->SetText(FText::GetEmpty());
        }
        return;
    }

    const TArray<FQuestProgress> ActiveQuests = QuestSubsystem->GetActiveQuests(ActiveCharacter);

    if (ActiveQuestNameText)
    {
        if (ActiveQuests.Num() > 0)
        {
            const FString& FirstQuestID = ActiveQuests[0].QuestID;
            FText Name = QuestSubsystem->GetQuestNameByID(FirstQuestID);
            const FString NameStr = Name.ToString();
            if (!NameStr.IsEmpty())
            {
                ActiveQuestNameText->SetText(Name);
            }
            else
            {
                ActiveQuestNameText->SetText(FText::FromString(FirstQuestID));
            }
        }
        else
        {
            ActiveQuestNameText->SetText(FText::GetEmpty());
        }
    }

    if (ActiveQuestTypeText)
    {
        if (ActiveQuests.Num() > 0)
        {
            const FString& FirstQuestID = ActiveQuests[0].QuestID;
            FText TypeText = QuestSubsystem->GetQuestTypeTextByID(FirstQuestID);
            ActiveQuestTypeText->SetText(TypeText);
        }
        else
        {
            ActiveQuestTypeText->SetText(FText::GetEmpty());
        }
    }
}

void UQuestWidget::HandleQuestAccepted(const FString& QuestID)
{
    UpdateActiveQuestCount();
}

void UQuestWidget::HandleQuestCompleted(const FString& QuestID)
{
    UpdateActiveQuestCount();
}

void UQuestWidget::HandleQuestFailed(const FString& QuestID)
{
    UpdateActiveQuestCount();
}

