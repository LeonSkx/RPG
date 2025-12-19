// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/RPGWidgetController.h"
#include "Progression/SkillTreeSubsystem.h"
#include "SkillTreeWidgetController.generated.h"

class ARPGCharacter;
class USkillTreeSubsystem;
class USkillEquipmentComponent;
class UDataTable;

/**
 * Controller para o widget de árvore de habilidades
 * Fornece a lógica de negócio para a UI de habilidades
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API USkillTreeWidgetController : public URPGWidgetController
{
    GENERATED_BODY()

public:
    USkillTreeWidgetController();

public:
    // === INICIALIZAÇÃO ===
    
    /** Inicializa o controller com um personagem específico */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    void InitializeWithCharacter(ARPGCharacter* Character);
    
    /** Atualiza o controller com um novo personagem */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    void UpdateCharacter(ARPGCharacter* NewCharacter);

    // === HABILIDADES ===
    
    /** Verifica se uma habilidade pode ser desbloqueada */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    bool CanUnlockSkill(const FName& SkillID) const;

    /** Verifica se uma habilidade pode ser desbloqueada por slot */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    bool CanUnlockSkillBySlot(const FName& SlotID) const;
    
    /** Desbloqueia uma habilidade */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    bool UnlockSkill(const FName& SkillID);

    /** Desbloqueia uma habilidade por slot */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    bool UnlockSkillBySlot(const FName& SlotID);
    
    /** Verifica se uma habilidade está desbloqueada */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    bool SkillUnlocked(const FName& SkillID) const;

    /** Verifica se uma habilidade está desbloqueada por slot */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    bool SkillUnlockedBySlot(const FName& SlotID) const;
    
    /** Obtém dados de uma habilidade */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    FSkillTreeNode GetSkillNode(const FName& SkillID) const;
    
    /** Obtém todas as habilidades disponíveis */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    TArray<FSkillTreeNode> GetAllAvailableSkills() const;
    
    /** Obtém habilidades desbloqueadas */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    TArray<FName> GetUnlockedSkills() const;
    
    /** Obtém habilidades que podem ser desbloqueadas */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    TArray<FSkillTreeNode> GetUnlockableSkills() const;

    // === EQUIPAMENTO ===
    
    /** Verifica se uma habilidade pode ser equipada em um slot */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    bool CanEquipSkill(const FName& SkillID, const FName& SlotID) const;
    
    /** Equipa uma habilidade em um slot */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    bool EquipSkill(const FName& SkillID, const FName& SlotID, const FGameplayTag& SlotInputTag);
    
    /** Desequipa uma habilidade de um slot */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    bool UnequipSkill(const FName& SlotID);
    
    /** Move uma habilidade de um slot para outro */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    bool MoveSkillToSlot(const FName& FromSlotID, const FName& ToSlotID);
    
    /** Troca habilidades entre slots */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    bool SwapSkillsBetweenSlots(const FName& SlotID1, const FName& SlotID2);
    
    /** Obtém a habilidade equipada em um slot */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    FName GetEquippedSkillInSlot(const FName& SlotID) const;
    
    /** Obtém o slot onde uma habilidade está equipada */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    FName GetSkillEquippedSlot(const FName& SkillID) const;
    
    /** Obtém todas as habilidades equipadas */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    TMap<FName, FName> GetAllEquippedSkills() const;

    // === PONTOS ===
    
    /** Obtém pontos de magia disponíveis */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    int32 GetSpellPoints() const;
    
    /** Adiciona pontos de magia */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    void AddSpellPoints(int32 Points);
    
    /** Gasta pontos de magia */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    bool SpendSpellPoints(int32 Points);

    // === SLOTS ===
    
    /** Obtém todos os slots ativos */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    TArray<FName> GetActiveSlotIDs() const;
    
    /** Verifica se um slot existe */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    bool HasSlot(const FName& SlotID) const;
    
    /** Obtém o número de slots ativos */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    int32 GetActiveSlotCount() const;

    // === UTILIDADES ===
    
    /** Obtém o personagem atual */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    ARPGCharacter* GetCurrentCharacter() const { return CurrentCharacter; }
    
    /** Obtém o SkillTreeSubsystem */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    USkillTreeSubsystem* GetSkillTreeSubsystem() const { return SkillTreeSubsystem; }
    
    /** Obtém o SkillEquipmentComponent */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    USkillEquipmentComponent* GetSkillEquipmentComponent() const { return SkillEquipmentComponent; }
    
    /** Obtém o Data Table de habilidades */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    UDataTable* GetSkillDataTable() const;

    // === MÉTODOS ADICIONAIS DO LEGACY ===

    /** Verifica se uma habilidade está bloqueada por slot */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    bool SkillLockedBySlot(const FName& SlotID) const;

    /** Verifica se uma habilidade está equipada por slot */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    bool SkillEquippedBySlot(const FName& SlotID) const;

    /** Retorna nível do personagem */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    int32 GetCharacterLevel() const;
    
    /** Retorna nome do personagem */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    FString GetCharacterName() const;

    /** Equipa uma habilidade por slot */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    bool EquipSkillBySlot(const FName& SlotID);
    
    /** Desequipa uma habilidade por slot */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    bool UnequipSkillBySlot(const FName& SlotID);

    /** Desbloqueia automaticamente habilidades sem pré-requisitos */
    UFUNCTION(BlueprintCallable, Category = "SkillTreeController")
    void UnlockSkillsWithoutPrerequisites();

    /** Retorna lista de habilidades equipadas */
    UFUNCTION(BlueprintPure, Category = "SkillTreeController")
    TArray<FName> GetEquippedSkills() const;

protected:
    // === DADOS INTERNOS ===
    
    /** Personagem atual */
    UPROPERTY(BlueprintReadOnly, Category = "SkillTreeController")
    ARPGCharacter* CurrentCharacter;
    
    /** SkillTreeSubsystem */
    UPROPERTY(BlueprintReadOnly, Category = "SkillTreeController")
    USkillTreeSubsystem* SkillTreeSubsystem;
    
    /** SkillEquipmentComponent do personagem */
    UPROPERTY(BlueprintReadOnly, Category = "SkillTreeController")
    USkillEquipmentComponent* SkillEquipmentComponent;
    
    /** Data Table de habilidades do personagem */
    UPROPERTY(BlueprintReadOnly, Category = "SkillTreeController")
    UDataTable* CharacterSkillDataTable;

    // === HELPERS ===
    
    /** Conecta aos eventos do personagem */
    void ConnectToCharacterEvents();
    
    /** Desconecta dos eventos do personagem */
    void DisconnectFromCharacterEvents();
    
    /** Callback quando uma habilidade é desbloqueada */
    UFUNCTION()
    void OnSkillUnlocked(ARPGCharacter* Character, FName SkillID);
    
    /** Callback quando uma habilidade é equipada */
    UFUNCTION()
    void OnSkillEquipped(FName CharacterID, FName SlotID, FName SkillID);
    
    /** Callback quando uma habilidade é desequipada */
    UFUNCTION()
    void OnSkillUnequipped(FName CharacterID, FName SlotID, FName SkillID);
    
    /** Callback quando pontos de magia mudam */
    UFUNCTION()
    void OnSpellPointsChanged();
    
    /** Atualiza o Data Table de habilidades */
    void UpdateSkillDataTable();
    
    /** Valida o personagem atual */
    bool ValidateCurrentCharacter() const;
};
