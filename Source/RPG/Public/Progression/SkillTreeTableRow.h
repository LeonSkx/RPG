// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SkillTreeTableRow.generated.h"

class UGameplayAbility;
class UTexture2D;

/**
 * Categorias de habilidades
 */
UENUM(BlueprintType)
enum class ESkillCategory : uint8
{
    Combat      UMETA(DisplayName = "Combate"),
    Magic       UMETA(DisplayName = "Magia"),
    Support     UMETA(DisplayName = "Suporte"),
    Passive     UMETA(DisplayName = "Passiva"),
    Ultimate    UMETA(DisplayName = "Ultimate")
};

/**
 * Struct para Data Table da árvore de habilidades
 * Substitui o Data Asset para suportar múltiplos personagens
 */
USTRUCT(BlueprintType)
struct RPG_API FSkillTreeTableRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FSkillTreeTableRow()
        : CharacterID(NAME_None)
        , SlotID(NAME_None)
        , SkillID(NAME_None)
        , SkillName(FText::FromString("Habilidade Desconhecida"))
        , SkillDescription(FText::FromString("Uma habilidade mágica poderosa"))
        , SkillIcon(nullptr)
        , AbilityClass(nullptr)
        , Category(ESkillCategory::Magic)
        , RequiredLevel(1)
        , RequiredSpellPoints(1)
        , Prerequisites()
        , RequiredQuestID("")
    {}

    // === IDENTIFICAÇÃO ===
    
    // @brief ID único do personagem
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Identification")
    FName CharacterID;

    // @brief ID do slot da habilidade
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Identification")
    FName SlotID;

    // @brief ID único da habilidade
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Identification")
    FName SkillID;

    // === VISUAL ===
    
    // @brief Nome da habilidade (suporta localização)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Visual")
    FText SkillName;

    // @brief Descrição da habilidade
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Visual")
    FText SkillDescription;

    // @brief Ícone da habilidade
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Visual")
    UTexture2D* SkillIcon;

    // === CONFIGURAÇÃO ===
    
    // @brief Classe da habilidade (GameplayAbility)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Configuration")
    TSubclassOf<UGameplayAbility> AbilityClass;

    // @brief Categoria da habilidade
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Configuration")
    ESkillCategory Category = ESkillCategory::Magic;



    // === PRÉ-REQUISITOS ===
    
    // @brief Nível necessário para desbloquear
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Requirements")
    int32 RequiredLevel;

    // @brief Pontos de magia necessários
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Requirements")
    int32 RequiredSpellPoints;

    // @brief IDs dos pré-requisitos (outras habilidades)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Requirements")
    TArray<FName> Prerequisites;

    // @brief ID da quest necessária para desbloquear (opcional)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill Tree|Requirements")
    FString RequiredQuestID;
}; 