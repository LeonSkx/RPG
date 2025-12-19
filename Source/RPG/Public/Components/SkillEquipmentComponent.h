// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillEquipmentComponent.generated.h"

class ARPGCharacter;
class USkillTreeSubsystem;

// Delegate para notificar mudanças de equipamento
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillEquipmentEquipped, FName, CharacterID, FName, SlotID, FName, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillEquipmentUnequipped, FName, CharacterID, FName, SlotID, FName, SkillID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSkillEquippedInSlot, FName, SlotID, FName, SkillID, UTexture2D*, SkillIcon);

/**
 * Componente dedicado para gerenciar equipamento de habilidades
 * Suporta múltiplos slots dinamicamente baseado em requests da UI
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API USkillEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USkillEquipmentComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === IDENTIFICAÇÃO ===
    
    /** Retorna o personagem dono deste componente */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    ARPGCharacter* GetOwnerCharacter() const;
    
    /** Retorna o CharacterID do personagem */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    FName GetCharacterID() const;
    
    // === EQUIPAMENTO ===
    
    /** Verifica se pode equipar uma habilidade em um slot específico */
    UFUNCTION(BlueprintCallable, Category = "SkillEquipment")
    bool CanEquipSkill(const FName& SkillID, const FName& SlotID) const;
    
    /** Equipa uma habilidade em um slot específico (cria o slot se não existir) */
    UFUNCTION(BlueprintCallable, Category = "SkillEquipment")
    bool EquipSkill(const FName& SkillID, const FName& SlotID, const FGameplayTag& SlotInputTag);
    
    /** Desequipa a habilidade de um slot específico */
    UFUNCTION(BlueprintCallable, Category = "SkillEquipment")
    bool UnequipSkill(const FName& SlotID);
    
    /** Desequipa todas as habilidades */
    UFUNCTION(BlueprintCallable, Category = "SkillEquipment")
    void UnequipAllSkills();
    
    /** Verifica se uma habilidade está equipada em algum slot */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    bool IsSkillEquipped(const FName& SkillID) const;
    
    /** Verifica se uma habilidade está equipada em um slot específico */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    bool IsSkillEquippedInSlot(const FName& SkillID, const FName& SlotID) const;
    
    /** Retorna o slot onde uma habilidade está equipada (NAME_None se não equipada) */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    FName GetSkillEquippedSlot(const FName& SkillID) const;
    
    /** Retorna a habilidade equipada em um slot (NAME_None se vazio) */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    FName GetEquippedSkillInSlot(const FName& SlotID) const;
    
    /** Retorna todas as habilidades equipadas (SlotID -> SkillID) */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    TMap<FName, FName> GetAllEquippedSkills() const;
    
    /** Verifica se um slot existe (tem alguma habilidade equipada) */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    bool HasSlot(const FName& SlotID) const;
    
    /** Retorna todos os SlotIDs que têm habilidades equipadas */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    TArray<FName> GetActiveSlotIDs() const;
    
    /** Move uma habilidade de um slot para outro */
    UFUNCTION(BlueprintCallable, Category = "SkillEquipment")
    bool MoveSkillToSlot(const FName& FromSlotID, const FName& ToSlotID);
    
    /** Troca habilidades entre dois slots */
    UFUNCTION(BlueprintCallable, Category = "SkillEquipment")
    bool SwapSkillsBetweenSlots(const FName& SlotID1, const FName& SlotID2);
    
    /** Obtém o ícone de uma habilidade */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    UTexture2D* GetSkillIcon(const FName& SkillID) const;
    
    /** Obtém o ícone da habilidade equipada em um slot específico */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    UTexture2D* GetSlotIcon(const FName& SlotID) const;
    
    /** Retorna o ícone padrão para slots vazios */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    UTexture2D* GetDefaultSlotIcon() const;
    
    // === UTILIDADES ===
    
    /** Retorna o número de slots ativos (com habilidades equipadas) */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    int32 GetActiveSlotCount() const { return EquippedSkills.Num(); }
    
    /** Retorna o número de habilidades equipadas */
    UFUNCTION(BlueprintPure, Category = "SkillEquipment")
    int32 GetEquippedSkillCount() const { return EquippedSkills.Num(); }
    
    // === DELEGATES ===
    
    /** Disparado quando uma habilidade é equipada */
    UPROPERTY(BlueprintAssignable, Category = "SkillEquipment")
    FOnSkillEquipmentEquipped OnSkillEquipmentEquipped;
    
    /** Disparado quando uma habilidade é desequipada */
    UPROPERTY(BlueprintAssignable, Category = "SkillEquipment")
    FOnSkillEquipmentUnequipped OnSkillEquipmentUnequipped;
    
    /** Disparado quando uma habilidade é equipada em um slot específico (com ícone) */
    UPROPERTY(BlueprintAssignable, Category = "SkillEquipment")
    FOnSkillEquippedInSlot OnSkillEquippedInSlot;
    
protected:
    // === DADOS INTERNOS ===
    
    /** Mapa de habilidades equipadas (SlotID -> SkillID) */
    UPROPERTY(BlueprintReadOnly, Category = "SkillEquipment")
    TMap<FName, FName> EquippedSkills;
    
    // === HELPERS ===
    
    /** Valida com o SkillTreeSubsystem */
    void ValidateWithSkillTreeSubsystem();
    
    /** Concede uma habilidade ao personagem via GAS */
    void GrantSkillToCharacter(const FName& SkillID, const FGameplayTag& SlotInputTag);
    
    /** Remove uma habilidade do personagem via GAS */
    void RemoveSkillFromCharacter(const FName& SkillID);
    
    // === EVENTOS ===
    
    /** Conecta aos eventos do SkillTreeSubsystem */
    void ConnectToSkillTreeEvents();
    
    /** Callback quando uma habilidade é desbloqueada */
    UFUNCTION()
    void OnSkillUnlocked(ARPGCharacter* Character, FName SkillID);
};
