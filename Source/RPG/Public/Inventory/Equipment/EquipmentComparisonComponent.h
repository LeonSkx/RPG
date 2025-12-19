#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Inventory/Core/InventoryEnums.h"
#include "Inventory/Core/InventoryTypes.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "GameplayEffectTypes.h"
#include "EquipmentComparisonComponent.generated.h"

// Estrutura de atributos principais
USTRUCT(BlueprintType)
struct FEquipmentStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float Attack = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float Armor = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MagicDamage = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float Accuracy = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MagicResist = 0.0f;
    
    // Flags para controlar se cada stat foi calculado
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    bool bAttackCalculated = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    bool bArmorCalculated = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    bool bMagicDamageCalculated = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    bool bAccuracyCalculated = false;
    
    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    bool bMagicResistCalculated = false;
    
    // Função helper para verificar se todos os stats foram calculados
    bool IsFullyCalculated() const
    {
        return bAttackCalculated && bArmorCalculated && 
               bMagicDamageCalculated && bAccuracyCalculated && 
               bMagicResistCalculated;
    }
    
    // Função helper para verificar se um stat específico foi calculado
    bool IsStatCalculated(int32 StatIndex) const
    {
        switch(StatIndex)
        {
            case 0: return bAttackCalculated;
            case 1: return bArmorCalculated;
            case 2: return bMagicDamageCalculated;
            case 3: return bAccuracyCalculated;
            case 4: return bMagicResistCalculated;
            default: return false;
        }
    }
};

class UEquippedItem;
class URPGAbilitySystemComponent;
class URPGAttributeSet;

// Componente para simular equipamentos sem equipar de fato
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPG_API UEquipmentComparisonComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEquipmentComparisonComponent();
    virtual void BeginPlay() override;

    // --- Simulação ---
    UFUNCTION(BlueprintCallable, Category="Equipment Comparison|Simulation")
    void SimulateEquipment(EEquipmentSlot Slot, const FInventoryItem& ItemToSimulate);

    UFUNCTION(BlueprintCallable, Category="Equipment Comparison|Simulation")
    void ClearSimulation(EEquipmentSlot Slot);

    UFUNCTION(BlueprintCallable, Category="Equipment Comparison|Simulation")
    void ClearAllSimulations();

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Simulation")
    bool HasSimulation(EEquipmentSlot Slot) const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Simulation")
    bool HasAnySimulation() const;

    // --- Valores atuais ---
    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Current Values")
    float GetCurrentAttack() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Current Values")
    float GetCurrentArmor() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Current Values")
    float GetCurrentMagicDamage() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Current Values")
    float GetCurrentAccuracy() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Current Values")
    float GetCurrentMagicResistance() const;

    // --- Valores simulados ---
    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Simulated Values")
    float GetSimulatedAttack() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Simulated Values")
    float GetSimulatedArmor() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Simulated Values")
    float GetSimulatedMagicDamage() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Simulated Values")
    float GetSimulatedAccuracy() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|Simulated Values")
    float GetSimulatedMagicResistance() const;

    // --- Texto para UI ---
    UFUNCTION(BlueprintPure, Category="Equipment Comparison|UI Text")
    FString GetAttackComparisonText() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|UI Text")
    FString GetArmorComparisonText() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|UI Text")
    FString GetMagicDamageComparisonText() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|UI Text")
    FString GetAccuracyComparisonText() const;

    UFUNCTION(BlueprintPure, Category="Equipment Comparison|UI Text")
    FString GetMagicResistanceComparisonText() const;

    // --- Auxiliares ---
    FString GetComparisonTextForStat(float Current, float Simulated) const;
    void ResetSimulatedStatsToDefault();
    void InvalidateCache();
    void ApplyItemEffectsToSimulatedStats(const UItemDataAsset* ItemData);



    // Delegates para notificar início/fim de simulação
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSimulationStartedSignature, EEquipmentSlot, Slot);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSimulationClearedSignature, EEquipmentSlot, Slot);

    UPROPERTY(BlueprintAssignable, Category="Equipment Comparison")
    FOnSimulationStartedSignature OnSimulationStarted;

    UPROPERTY(BlueprintAssignable, Category="Equipment Comparison")
    FOnSimulationClearedSignature OnSimulationCleared;

    // === CONFIGURAÇÕES DO SISTEMA ===
    
    // Tolerância para considerar valores como neutros (configurável)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equipment Comparison|Settings", meta=(ClampMin="0.001", ClampMax="1.0"))
    float ToleranceForNeutral = 0.01f;
    
    // Precisão decimal para diferentes tipos de stats (configurável)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equipment Comparison|Settings", meta=(ClampMin="0", ClampMax="3"))
    int32 AttackDecimals = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equipment Comparison|Settings", meta=(ClampMin="0", ClampMax="3"))
    int32 ArmorDecimals = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equipment Comparison|Settings", meta=(ClampMin="0", ClampMax="3"))
    int32 MagicDamageDecimals = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equipment Comparison|Settings", meta=(ClampMin="0", ClampMax="3"))
    int32 AccuracyDecimals = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equipment Comparison|Settings", meta=(ClampMin="0", ClampMax="3"))
    int32 MagicResistDecimals = 0;

protected:
    void CalculateSimulatedStats();
    URPGAbilitySystemComponent* GetAbilitySystemComponent() const;
    float GetCurrentAttributeValue(FGameplayAttribute Attribute) const;

    void SimulateItemGameplayEffects(const UItemDataAsset* ItemData);
    void SimulateGameplayEffect(TSubclassOf<UGameplayEffect> EffectClass, int32 Level, URPGAbilitySystemComponent* ASC);

    float CalculateModifierValue(const FGameplayModifierInfo& Modifier, int32 Level, URPGAbilitySystemComponent* ASC);
    float CalculateModifierValueFromSpec(const FGameplayModifierInfo& Modifier, const FGameplayEffectSpec* Spec, int32 Level, URPGAbilitySystemComponent* ASC);

    void ApplySimulatedModifier(FGameplayAttribute Attribute, float Value, EGameplayModOp::Type Operation);
    void ApplyModifierToStat(float& SimulatedStat, float OriginalValue, float ModifierValue, EGameplayModOp::Type Operation, const FString& StatName);

    FString GetSmartComparisonText(float Current, float Simulated, const FString& StatName) const;

    void CalculateCurrentStats();
    void CalculateSimulatedStats(const UItemDataAsset* ItemToSimulate);

    void ApplyModifierToStats(FEquipmentStats& Stats, const FGameplayAttribute& Attribute, float Value);
    FEquipmentStats GetBaseStatsWithoutEquipment() const;
    void ApplyCurrentEquipmentEffectsExceptSlot();
    bool IsItemAlreadyEquipped(const UItemDataAsset* ItemToCheck) const;

private:
    // Itens e simulações ativas
    UPROPERTY(BlueprintReadOnly, Category="Equipment Comparison", meta=(AllowPrivateAccess="true"))
    TMap<EEquipmentSlot, FInventoryItem> SimulatedItems;

    UPROPERTY(BlueprintReadOnly, Category="Equipment Comparison", meta=(AllowPrivateAccess="true"))
    TMap<EEquipmentSlot, bool> ActiveSimulations;

    // Valores simulados
    UPROPERTY(BlueprintReadOnly, Category="Equipment Comparison", meta=(AllowPrivateAccess="true"))
    float SimulatedAttack = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Equipment Comparison", meta=(AllowPrivateAccess="true"))
    float SimulatedArmor = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Equipment Comparison", meta=(AllowPrivateAccess="true"))
    float SimulatedMagicDamage = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Equipment Comparison", meta=(AllowPrivateAccess="true"))
    float SimulatedAccuracy = 0.f;

    UPROPERTY(BlueprintReadOnly, Category="Equipment Comparison", meta=(AllowPrivateAccess="true"))
    float SimulatedMagicResistance = 0.f;

    // Cache de valores atuais
    float CachedCurrentAttack = 0.f;
    float CachedCurrentArmor = 0.f;
    float CachedCurrentMagicDamage = 0.f;
    float CachedCurrentAccuracy = 0.f;
    float CachedCurrentMagicResistance = 0.f;

    bool bCurrentValuesCached = false;

    // Estrutura de stats atual e simulado
    UPROPERTY(BlueprintReadOnly, Category="Equipment Comparison", meta=(AllowPrivateAccess="true"))
    FEquipmentStats CurrentStats;

    UPROPERTY(BlueprintReadOnly, Category="Equipment Comparison", meta=(AllowPrivateAccess="true"))
    FEquipmentStats SimulatedStats;

    // Funções privadas
    void UpdateCurrentValuesCache();
    FString FormatComparisonText(float CurrentValue, float SimulatedValue, const FString& Label) const;
    bool IsItemCompatibleWithSlot(const FInventoryItem& Item, EEquipmentSlot Slot) const;
    bool IsItemCurrentlyEquipped(EEquipmentSlot Slot, const FInventoryItem& Item) const;
};
