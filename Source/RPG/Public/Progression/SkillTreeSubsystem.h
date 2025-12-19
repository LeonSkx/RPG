// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "SkillTreeSubsystem.generated.h"

class ARPGCharacter;
class UGameplayAbility;
class UTexture2D;

// Delegate para habilidade desbloqueada
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillUnlocked, ARPGCharacter*, Character, FName, SkillID);

// REMOVIDO: Delegates de equipamento movidos para SkillEquipmentComponent

// Delegate para mudança de pontos (global)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPointsChanged);

/**
 * Estrutura para um nó da árvore de habilidades
 */
USTRUCT(BlueprintType)
struct FSkillTreeNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    FName CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    FName SlotID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    FName SkillID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    TSubclassOf<UGameplayAbility> AbilityClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    int32 RequiredLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    int32 RequiredSpellPoints = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    TArray<FName> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    FText SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    FText SkillDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill Tree")
    UTexture2D* SkillIcon;

    FSkillTreeNode()
        : CharacterID(NAME_None)
        , SlotID(NAME_None)
        , SkillID(NAME_None)
        , AbilityClass(nullptr)
        , RequiredLevel(1)
        , RequiredSpellPoints(1)
        , SkillName(FText::FromString("Habilidade Desconhecida"))
        , SkillDescription(FText::FromString("Uma habilidade mágica poderosa"))
        , SkillIcon(nullptr)
    {
    }
};



/**
 * Subsistema para gerenciar a árvore de habilidades de todos os personagens.
 * Controla desbloqueio, pré-requisitos e concessão de habilidades.
 */
UCLASS()
class RPG_API USkillTreeSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // === CONFIGURAÇÃO ===
    
    /** Define o Data Table da árvore de habilidades (mantém compatibilidade) */
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    void SetSkillTreeDataTable(UDataTable* NewSkillTreeDataTable);

    /** Define o Data Table específico para um personagem */
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    void SetCharacterSkillTable(const FName& CharacterID, UDataTable* DataTable);

    /** Obtém o Data Table da árvore de habilidades (mantém compatibilidade) */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    UDataTable* GetSkillTreeDataTable() const { return SkillTreeDataTable; }

    /** Obtém dados de uma habilidade específica */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    FSkillTreeNode GetSkillData(const FName& CharacterID, const FName& SkillID) const;



    /** Verifica se o requisito de quest foi atendido */
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    bool CheckQuestRequirement(const FString& RequiredQuestID, ARPGCharacter* Character) const;

    /** Obtém o Data Table específico de um personagem */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    UDataTable* GetCharacterSkillTable(const FName& CharacterID) const;

    // === VERIFICAÇÕES ===
    
    /** Verifica se uma habilidade pode ser desbloqueada */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    bool CanUnlockSkill(const ARPGCharacter* Character, const FName& SkillID) const;

    /** Verifica se uma habilidade pode ser desbloqueada por slot */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    bool CanUnlockSkillBySlot(const ARPGCharacter* Character, const FName& SlotID) const;

    /** Verifica se uma habilidade já está desbloqueada */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    bool SkillUnlocked(const ARPGCharacter* Character, const FName& SkillID) const;

    /** Verifica se uma habilidade já está desbloqueada por slot */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    bool SkillUnlockedBySlot(const ARPGCharacter* Character, const FName& SlotID) const;

    /** Verifica se uma habilidade está bloqueada por slot */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    bool SkillLockedBySlot(const ARPGCharacter* Character, const FName& SlotID) const;

    /** Verifica se todos os pré-requisitos de uma habilidade estão atendidos */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    bool ArePrerequisitesMet(const ARPGCharacter* Character, const FName& SkillID) const;

    // === AÇÕES ===
    
    /** Desbloqueia uma habilidade (consome pontos de magia) */
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    bool UnlockSkill(ARPGCharacter* Character, const FName& SkillID);

    /** Desbloqueia uma habilidade por slot (consome pontos de magia) */
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    bool UnlockSkillBySlot(ARPGCharacter* Character, const FName& SlotID);

    /** Força o desbloqueio de uma habilidade (sem consumir pontos) */
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    bool ForceUnlockSkill(ARPGCharacter* Character, const FName& SkillID);
    
    /** Evento disparado quando pontos mudam */
    UPROPERTY(BlueprintAssignable, Category = "SkillTree")
    FOnPointsChanged OnPointsChanged;
    
    /** Notifica que pontos mudaram (para disparar delegate) */
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    void NotifyPointsChanged();

    // === CONSULTAS ===
    
    /** Retorna lista de habilidades desbloqueadas do personagem */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    TArray<FName> GetUnlockedSkills(const ARPGCharacter* Character) const;

    /** Retorna lista de habilidades que podem ser desbloqueadas */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    TArray<FName> GetAvailableSkills(const ARPGCharacter* Character) const;

    /** Obtém dados de um nó específico da árvore */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    FSkillTreeNode GetSkillNode(const FName& SkillID) const;

    /** Obtém dados de um nó específico por CharacterID e SlotID */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    FSkillTreeNode GetSkillNodeBySlot(const FName& CharacterID, const FName& SlotID) const;

    /** Obtém dados de uma linha do Data Table por CharacterID e SlotID */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    FSkillTreeTableRow GetSkillTableRowBySlot(const FName& CharacterID, const FName& SlotID) const;

    /** Obtém o número de habilidades desbloqueadas */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    int32 GetUnlockedSkillCount(const ARPGCharacter* Character) const;

    /** Lista todos os SkillIDs disponíveis no Data Asset */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    TArray<FName> GetAvailableSkillIDs() const;

    /** Verifica se o Data Asset está conectado */
    UFUNCTION(BlueprintPure, Category = "SkillTree")
    bool SkillTreeDataConnected() const;

    /** Desbloqueia automaticamente habilidades sem pré-requisitos */
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    void UnlockSkillsWithoutPrerequisites(ARPGCharacter* Character);

    // === SISTEMA DE EQUIPAMENTO ===
    // REMOVIDO: Funções de equipamento movidas para SkillEquipmentComponent
    // O SkillTreeSubsystem agora foca apenas em desbloqueio e dados

    // === EVENTOS ===
    
    /** Evento disparado quando uma habilidade é desbloqueada */
    UPROPERTY(BlueprintAssignable, Category = "SkillTree")
    FOnSkillUnlocked OnSkillUnlocked;
    
    // REMOVIDO: Eventos de equipamento movidos para SkillEquipmentComponent

private:
    /** Data Table da árvore de habilidades (mantém compatibilidade) */
    UPROPERTY()
    TObjectPtr<UDataTable> SkillTreeDataTable;

    /** Mapa de Data Tables por personagem */
    TMap<FName, TObjectPtr<UDataTable>> CharacterSkillTables;

    /** Mapa de habilidades desbloqueadas por personagem */
    TMap<FName, TSet<FName>> CharacterUnlockedSkills;
    

    
    // REMOVIDO: CharacterEquippedSkills movido para SkillEquipmentComponent

    /** Garante que dados do personagem existam */
    void EnsureCharacterDataExists(const ARPGCharacter* Character);

    /** Concede a habilidade ao personagem via GAS */
    void GrantSkillToCharacter(ARPGCharacter* Character, const FName& SkillID);
    
    /** Obtém a classe da habilidade (helper) */
    TSubclassOf<UGameplayAbility> GetSkillAbilityClass(const ARPGCharacter* Character, const FName& SkillID) const;
}; 