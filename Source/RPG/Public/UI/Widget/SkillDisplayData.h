// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Progression/SkillTreeTableRow.h"
#include "SkillDisplayData.generated.h"

class UTexture2D;

/**
 * Estados possíveis de uma habilidade
 */
UENUM(BlueprintType)
enum class ESkillState : uint8
{
    Locked      UMETA(DisplayName = "Bloqueada"),
    Unlocked    UMETA(DisplayName = "Desbloqueada"),
    Equipped    UMETA(DisplayName = "Equipada")
};

/**
 * Struct para armazenar dados de exibição de uma habilidade
 */
USTRUCT(BlueprintType)
struct RPG_API FSkillDisplayData
{
    GENERATED_BODY()

public:
    FSkillDisplayData()
        : SkillName(FText::FromString("Habilidade Desconhecida"))
        , SkillCost(0)
        , SkillDescription(FText::FromString("Descrição não disponível"))
        , bLocked(true)
        , bUnlocked(false)
        , bEquipped(false)
        , bCanUnlock(false)
        , SkillState(ESkillState::Locked)
        , SkillIcon(nullptr)
        , Category(ESkillCategory::Magic)
        , Prerequisites()
        , RequiredQuestID("")
    {}

    // @brief Nome da habilidade
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    FText SkillName;

    // @brief Custo em pontos da habilidade
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    int32 SkillCost;

    // @brief Descrição da habilidade
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    FText SkillDescription;

    // @brief Se a habilidade está bloqueada
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    bool bLocked;

    // @brief Se a habilidade está desbloqueada
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    bool bUnlocked;

    // @brief Se a habilidade está equipada
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    bool bEquipped;

    // @brief Se pode desbloquear a habilidade
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    bool bCanUnlock;

    // @brief Estado da habilidade
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    ESkillState SkillState;

    // @brief Ícone da habilidade
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    UTexture2D* SkillIcon;



    // @brief Categoria da habilidade
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    ESkillCategory Category;

    // @brief Lista de pré-requisitos (para exibição na UI)
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    TArray<FName> Prerequisites;

    // @brief ID da quest necessária para desbloquear
    UPROPERTY(BlueprintReadWrite, Category = "SkillDisplay")
    FString RequiredQuestID;
}; 