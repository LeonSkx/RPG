#pragma once

#include "CoreMinimal.h"
#include "QuestTypes.generated.h"

// === ENUMS ===

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Main        UMETA(DisplayName = "Main Story"),
    Side        UMETA(DisplayName = "Side Story"),
    Character   UMETA(DisplayName = "Character Quest"),
    World       UMETA(DisplayName = "World Quest"),
    Daily       UMETA(DisplayName = "Daily Quest"),
    Tutorial    UMETA(DisplayName = "Tutorial")
};

UENUM(BlueprintType)
enum class EQuestState : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Abandoned   UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
    Kill        UMETA(DisplayName = "Kill Enemy"),
    Collect     UMETA(DisplayName = "Collect Item"),
    Talk        UMETA(DisplayName = "Talk to NPC"),
    Reach       UMETA(DisplayName = "Reach Location"),
    Use         UMETA(DisplayName = "Use Item/Ability")
};

// === STRUCTS: OBJECTIVE, REWARD, PREREQUISITE ===

USTRUCT(BlueprintType)
struct FQuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOptional = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredObjectives;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentAmount = 0;

    UPROPERTY(BlueprintReadWrite)
    bool bIsCompleted = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsAvailable = true;
};

USTRUCT(BlueprintType)
struct FQuestRewards
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Experience = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Gold = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AttributePoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpellPoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShareRewardsWithGroup = true;
};

USTRUCT(BlueprintType)
struct FQuestPrerequisites
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCheckPartyAverageLevel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredItems;
};

// === STRUCTS: QUEST DATA & PROGRESS ===

USTRUCT(BlueprintType)
struct FQuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = "true"))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FQuestPrerequisites Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FQuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FQuestRewards Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeAbandoned = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeRepeated = false;
};

USTRUCT(BlueprintType)
struct FQuestProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString QuestID;

    UPROPERTY(BlueprintReadWrite)
    EQuestState State = EQuestState::NotStarted;

    UPROPERTY(BlueprintReadWrite)
    TArray<FQuestObjective> ObjectiveProgress;

    UPROPERTY(BlueprintReadWrite)
    bool bRewardsClaimed = false;
};
