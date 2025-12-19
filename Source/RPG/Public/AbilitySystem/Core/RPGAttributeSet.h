// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "RPGAbilityTypes.h"
#include "RPGAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Attribute Set customizado para o RPG
 * 
 * ESTRUTURA SIMPLIFICADA:
 * - Combat Core (5 atributos exibidos na UI)
 * - Primary Attributes (4 atributos básicos)
 * - Vital Attributes (Vida e Mana)
 * - Meta Attributes (para lógica de gameplay)
 * - Secondary Attributes removidos para simplificar o sistema
 */
UCLASS()
class RPG_API URPGAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    URPGAttributeSet();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Meta Attributes execution handling
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

    // ========================================
    // ATRIBUTOS EXIBIDOS NA UI (5 CORE)
    // ========================================
    
    // Combat Core Attributes
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Attack, Category = "Combat Core")
    FGameplayAttributeData Attack; // Ataque físico base
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, Attack);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MagicDamage, Category = "Combat Core")
    FGameplayAttributeData MagicDamage; // Dano mágico/arcano
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, MagicDamage);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Accuracy, Category = "Combat Core")
    FGameplayAttributeData Accuracy; // Precisão, chance de acerto
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, Accuracy);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Combat Core")
    FGameplayAttributeData Armor; // Defesa física
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, Armor);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MagicResistance, Category = "Combat Core")
    FGameplayAttributeData MagicResistance; // Defesa mágica
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, MagicResistance);

    // ========================================
    // MECÂNICAS INTERNAS (não exibidas na UI)
    // ========================================

    // Primary Attributes (para cálculos internos)
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
    FGameplayAttributeData Strength;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, Strength);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Primary Attributes")
    FGameplayAttributeData Intelligence;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, Intelligence);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Resilience, Category = "Primary Attributes")
    FGameplayAttributeData Resilience;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, Resilience);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Vigor, Category = "Primary Attributes")
    FGameplayAttributeData Vigor;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, Vigor);

    // Secondary Attributes (para cálculos internos)
    // Removidos para simplificar o sistema - mecânicas especiais virão de equipamentos/habilidades

    // Vital Attributes (para mecânicas de sobrevivência)
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, Health);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Vital Attributes")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, MaxHealth);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Vital Attributes")
    FGameplayAttributeData Mana;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, Mana);

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Vital Attributes")
    FGameplayAttributeData MaxMana;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, MaxMana);

    // Energy removido - Mana já serve para recursos

    // Meta Attributes (para lógica de gameplay)
    UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
    FGameplayAttributeData IncomingDamage;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, IncomingDamage);

    UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
    FGameplayAttributeData IncomingXP;
    ATTRIBUTE_ACCESSORS(URPGAttributeSet, IncomingXP);

    // ========================================
    // FUNÇÕES OnRep PARA REPLICAÇÃO
    // ========================================

    // Combat Core OnRep Functions
    UFUNCTION()
    void OnRep_Attack(const FGameplayAttributeData& OldAttack) const;
    UFUNCTION()
    void OnRep_MagicDamage(const FGameplayAttributeData& OldMagicDamage) const;
    UFUNCTION()
    void OnRep_Accuracy(const FGameplayAttributeData& OldAccuracy) const;
    UFUNCTION()
    void OnRep_Armor(const FGameplayAttributeData& OldArmor) const;
    UFUNCTION()
    void OnRep_MagicResistance(const FGameplayAttributeData& OldMagicResistance) const;

    // Primary Attributes OnRep Functions
    UFUNCTION()
    void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;
    UFUNCTION()
    void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;
    UFUNCTION()
    void OnRep_Resilience(const FGameplayAttributeData& OldResilience) const;
    UFUNCTION()
    void OnRep_Vigor(const FGameplayAttributeData& OldVigor) const;

    // Secondary Attributes OnRep Functions - Removidos

    // Vital Attributes OnRep Functions
    UFUNCTION()
    void OnRep_Health(const FGameplayAttributeData& OldHealth) const;
    UFUNCTION()
    void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;
    UFUNCTION()
    void OnRep_Mana(const FGameplayAttributeData& OldMana) const;
    UFUNCTION()
    void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

private:

	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props) const;

    
    void SendXPEvent(const FEffectProperties& Props);
}; 