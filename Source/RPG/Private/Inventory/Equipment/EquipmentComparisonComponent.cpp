#include "Inventory/Equipment/EquipmentComparisonComponent.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Character/RPGCharacter.h"
#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquippedItem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/Engine.h"

UEquipmentComparisonComponent::UEquipmentComparisonComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEquipmentComparisonComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UEquipmentComparisonComponent::SimulateEquipment(EEquipmentSlot Slot, const FInventoryItem& ItemToSimulate)
{
    if (!ItemToSimulate.IsValid() || !IsItemCompatibleWithSlot(ItemToSimulate, Slot))
        return;



    if (ActiveSimulations.Contains(Slot) && ActiveSimulations[Slot])
        ClearSimulation(Slot);

    SimulatedItems.Add(Slot, ItemToSimulate);
    ActiveSimulations.Add(Slot, true);

    CalculateSimulatedStats();
    OnSimulationStarted.Broadcast(Slot);
}

void UEquipmentComparisonComponent::ClearSimulation(EEquipmentSlot Slot)
{
    if (!ActiveSimulations.Contains(Slot) || !ActiveSimulations[Slot])
        return;

    SimulatedItems.Remove(Slot);
    ActiveSimulations.Remove(Slot);

    CalculateSimulatedStats();
    OnSimulationCleared.Broadcast(Slot);
}

void UEquipmentComparisonComponent::ClearAllSimulations()
{
    if (ActiveSimulations.Num() == 0) return;

    SimulatedItems.Empty();
    ActiveSimulations.Empty();

    SimulatedAttack = SimulatedArmor = SimulatedMagicDamage = SimulatedAccuracy = SimulatedMagicResistance = 0.f;
}

bool UEquipmentComparisonComponent::HasSimulation(EEquipmentSlot Slot) const
{
    return ActiveSimulations.Contains(Slot) && ActiveSimulations[Slot];
}

bool UEquipmentComparisonComponent::HasAnySimulation() const
{
    return ActiveSimulations.Num() > 0;
}

// === VALORES ATUAIS ===

float UEquipmentComparisonComponent::GetCurrentAttack() const
{
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    return ASC ? ASC->GetNumericAttribute(URPGAttributeSet::GetAttackAttribute()) : 0.f;
}

float UEquipmentComparisonComponent::GetCurrentArmor() const
{
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    return ASC ? ASC->GetNumericAttribute(URPGAttributeSet::GetArmorAttribute()) : 0.f;
}

float UEquipmentComparisonComponent::GetCurrentMagicDamage() const
{
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    return ASC ? ASC->GetNumericAttribute(URPGAttributeSet::GetMagicDamageAttribute()) : 0.f;
}

float UEquipmentComparisonComponent::GetCurrentAccuracy() const
{
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    return ASC ? ASC->GetNumericAttribute(URPGAttributeSet::GetAccuracyAttribute()) : 0.f;
}

float UEquipmentComparisonComponent::GetCurrentMagicResistance() const
{
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    return ASC ? ASC->GetNumericAttribute(URPGAttributeSet::GetMagicResistanceAttribute()) : 0.f;
}

// === VALORES SIMULADOS ===

float UEquipmentComparisonComponent::GetSimulatedAttack() const { 
    return HasAnySimulation() ? SimulatedAttack : GetCurrentAttack(); 
}

float UEquipmentComparisonComponent::GetSimulatedArmor() const { 
    return HasAnySimulation() ? SimulatedArmor : GetCurrentArmor(); 
}

float UEquipmentComparisonComponent::GetSimulatedMagicDamage() const { 
    return HasAnySimulation() ? SimulatedMagicDamage : GetCurrentMagicDamage(); 
}

float UEquipmentComparisonComponent::GetSimulatedAccuracy() const { 
    return HasAnySimulation() ? SimulatedAccuracy : GetCurrentAccuracy(); 
}

float UEquipmentComparisonComponent::GetSimulatedMagicResistance() const { 
    return HasAnySimulation() ? SimulatedMagicResistance : GetCurrentMagicResistance(); 
}

FString UEquipmentComparisonComponent::GetAttackComparisonText() const 
{ 
    if (HasAnySimulation())
    {
        float CurrentAttackValue = GetCurrentAttack();
        float SimulatedAttackValue = GetSimulatedAttack();
        float Delta = SimulatedAttackValue - CurrentAttackValue;
        
        if (FMath::IsNearlyZero(Delta, ToleranceForNeutral))
            return FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentAttackValue)); // Neutro
        
        if (Delta > 0)
            return FString::Printf(TEXT("%d ↑"), 
                FMath::FloorToInt(SimulatedAttackValue));
        else
            return FString::Printf(TEXT("%d ↓"), 
                FMath::FloorToInt(SimulatedAttackValue));
    }
    return FString::Printf(TEXT("%d"), FMath::FloorToInt(GetCurrentAttack())); 
}

FString UEquipmentComparisonComponent::GetArmorComparisonText() const 
{ 
    if (HasAnySimulation())
    {
        float CurrentArmorValue = GetCurrentArmor();
        float SimulatedArmorValue = GetSimulatedArmor();
        float Delta = SimulatedArmorValue - CurrentArmorValue;
        
        if (FMath::IsNearlyZero(Delta, ToleranceForNeutral))
            return FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentArmorValue)); // Neutro
        
        if (Delta > 0)
            return FString::Printf(TEXT("%d ↑"), 
                FMath::FloorToInt(SimulatedArmorValue));
        else
            return FString::Printf(TEXT("%d ↓"), 
                FMath::FloorToInt(SimulatedArmorValue));
    }
    return FString::Printf(TEXT("%d"), FMath::FloorToInt(GetCurrentArmor())); 
}

FString UEquipmentComparisonComponent::GetMagicDamageComparisonText() const 
{ 
    if (HasAnySimulation())
    {
        float CurrentMagicDamageValue = GetCurrentMagicDamage();
        float SimulatedMagicDamageValue = GetSimulatedMagicDamage();
        float Delta = SimulatedMagicDamageValue - CurrentMagicDamageValue;
        
        if (FMath::IsNearlyZero(Delta, ToleranceForNeutral))
            return FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentMagicDamageValue)); // Neutro
        
        if (Delta > 0)
            return FString::Printf(TEXT("%d ↑"), 
                FMath::FloorToInt(SimulatedMagicDamageValue));
        else
            return FString::Printf(TEXT("%d ↓"), 
                FMath::FloorToInt(SimulatedMagicDamageValue));
    }
    return FString::Printf(TEXT("%d"), FMath::FloorToInt(GetCurrentMagicDamage())); 
}

FString UEquipmentComparisonComponent::GetAccuracyComparisonText() const 
{ 
    if (HasAnySimulation())
    {
        float CurrentAccuracyValue = GetCurrentAccuracy();
        float SimulatedAccuracyValue = GetSimulatedAccuracy();
        float Delta = SimulatedAccuracyValue - CurrentAccuracyValue;
        
        if (FMath::IsNearlyZero(Delta, ToleranceForNeutral))
            return FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentAccuracyValue)); // Neutro
        
        if (Delta > 0)
            return FString::Printf(TEXT("%d ↑"), 
                FMath::FloorToInt(SimulatedAccuracyValue));
        else
            return FString::Printf(TEXT("%d ↓"), 
                FMath::FloorToInt(SimulatedAccuracyValue));
    }
    return FString::Printf(TEXT("%d"), FMath::FloorToInt(GetCurrentAccuracy())); 
}

FString UEquipmentComparisonComponent::GetMagicResistanceComparisonText() const 
{ 
    if (HasAnySimulation())
    {
        float CurrentMagicResistanceValue = GetCurrentMagicResistance();
        float SimulatedMagicResistanceValue = GetSimulatedMagicResistance();
        float Delta = SimulatedMagicResistanceValue - CurrentMagicResistanceValue;
        
        if (FMath::IsNearlyZero(Delta, ToleranceForNeutral))
            return FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentMagicResistanceValue)); // Neutro
        
        if (Delta > 0)
            return FString::Printf(TEXT("%d ↑"), 
                FMath::FloorToInt(SimulatedMagicResistanceValue));
        else
            return FString::Printf(TEXT("%d ↓"), 
                FMath::FloorToInt(SimulatedMagicResistanceValue));
    }
    return FString::Printf(TEXT("%d"), FMath::FloorToInt(GetCurrentMagicResistance())); 
}

void UEquipmentComparisonComponent::CalculateSimulatedStats()
{
    if (!HasAnySimulation())
    {
        SimulatedAttack = GetCurrentAttack();
        SimulatedArmor = GetCurrentArmor();
        SimulatedMagicDamage = GetCurrentMagicDamage();
        SimulatedAccuracy = GetCurrentAccuracy();
        SimulatedMagicResistance = GetCurrentMagicResistance();
        return;
    }

    // NOVO SISTEMA: Calcular stats estruturados
    CalculateCurrentStats();
    
    // Para cada item simulado, calcular os stats simulados
    for (auto& Pair : SimulatedItems)
    {
        if (Pair.Value.IsValid() && Pair.Value.ItemData)
        {
            CalculateSimulatedStats(Pair.Value.ItemData);
            break; // Por enquanto, apenas um item por vez
        }
    }
    
    // Atualizar os valores legados para compatibilidade
    SimulatedAttack = SimulatedStats.Attack;
    SimulatedArmor = SimulatedStats.Armor;
    SimulatedMagicDamage = SimulatedStats.MagicDamage;
    SimulatedAccuracy = SimulatedStats.Accuracy;
    SimulatedMagicResistance = SimulatedStats.MagicResist;
}

void UEquipmentComparisonComponent::CalculateCurrentStats()
{
    CurrentStats = FEquipmentStats();
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;

    // Usar o novo atributo Attack diretamente
    CurrentStats.Attack = ASC->GetNumericAttribute(URPGAttributeSet::GetAttackAttribute());
    CurrentStats.Armor = ASC->GetNumericAttribute(URPGAttributeSet::GetArmorAttribute());
    CurrentStats.MagicDamage = ASC->GetNumericAttribute(URPGAttributeSet::GetMagicDamageAttribute());
    CurrentStats.Accuracy = ASC->GetNumericAttribute(URPGAttributeSet::GetAccuracyAttribute());
    CurrentStats.MagicResist = ASC->GetNumericAttribute(URPGAttributeSet::GetMagicResistanceAttribute());
}

void UEquipmentComparisonComponent::CalculateSimulatedStats(const UItemDataAsset* ItemToSimulate)
{
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ItemToSimulate || !ASC) return;

    // VERIFICAR SE É O MESMO ITEM JÁ EQUIPADO
    if (IsItemAlreadyEquipped(ItemToSimulate))
    {
        // MESMO ITEM: Stats simulados = stats atuais (neutro)
        SimulatedStats = CurrentStats;
        return;
    }

    // ITEM DIFERENTE: COMEÇAR COM STATS BASE (sem equipamentos)
    SimulatedStats = GetBaseStatsWithoutEquipment();

    // APLICAR TODOS OS EQUIPAMENTOS ATUAIS EXCETO O SLOT SENDO SIMULADO
    ApplyCurrentEquipmentEffectsExceptSlot();

    // APLICAR EFEITOS DO NOVO ITEM
    for (const FEquipmentGameplayEffect& EffectData : ItemToSimulate->EquipmentGameplayEffects)
    {
        if (EffectData.GameplayEffectClass)
        {
            const UGameplayEffect* GE = EffectData.GameplayEffectClass->GetDefaultObject<UGameplayEffect>();
            if (!GE) continue;

            FGameplayEffectSpec Spec(GE, ASC->MakeEffectContext(), EffectData.EffectLevel);
            for (const FGameplayModifierInfo& Mod : GE->Modifiers)
            {
                float Value = 0.0f;
                if (Mod.ModifierMagnitude.AttemptCalculateMagnitude(Spec, Value))
                {
                    ApplyModifierToStats(SimulatedStats, Mod.Attribute, Value);
                }
            }
        }
    }
}

void UEquipmentComparisonComponent::ApplyModifierToStats(FEquipmentStats& Stats, const FGameplayAttribute& Attribute, float Value)
{
    if (Attribute == URPGAttributeSet::GetAttackAttribute())
        Stats.Attack += Value;  // ✅ MANTÉM PRECISÃO durante cálculo
    else if (Attribute == URPGAttributeSet::GetArmorAttribute())
        Stats.Armor += Value;   // ✅ MANTÉM PRECISÃO durante cálculo
    else if (Attribute == URPGAttributeSet::GetMagicDamageAttribute())
        Stats.MagicDamage += Value;  // ✅ MANTÉM PRECISÃO durante cálculo
    else if (Attribute == URPGAttributeSet::GetAccuracyAttribute())
        Stats.Accuracy += Value;     // ✅ MANTÉM PRECISÃO durante cálculo
    else if (Attribute == URPGAttributeSet::GetMagicResistanceAttribute())
        Stats.MagicResist += Value;  // ✅ MANTÉM PRECISÃO durante cálculo
}

FEquipmentStats UEquipmentComparisonComponent::GetBaseStatsWithoutEquipment() const
{
    FEquipmentStats BaseStats;
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return BaseStats;

    // Obter valores atuais dos atributos (com equipamentos)
    BaseStats.Attack = ASC->GetNumericAttribute(URPGAttributeSet::GetAttackAttribute());
    BaseStats.Armor = ASC->GetNumericAttribute(URPGAttributeSet::GetArmorAttribute());
    BaseStats.MagicDamage = ASC->GetNumericAttribute(URPGAttributeSet::GetMagicDamageAttribute());
    BaseStats.Accuracy = ASC->GetNumericAttribute(URPGAttributeSet::GetAccuracyAttribute());
    BaseStats.MagicResist = ASC->GetNumericAttribute(URPGAttributeSet::GetMagicResistanceAttribute());
    
    // Subtrair os efeitos de todos os equipamentos atuais
    if (AActor* Owner = GetOwner())
    {
        if (UEquipmentComponent* EquipmentComp = Owner->FindComponentByClass<UEquipmentComponent>())
        {
            for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
            {
                EEquipmentSlot Slot = static_cast<EEquipmentSlot>(SlotIndex);
                if (UEquippedItem* EquippedItem = EquipmentComp->GetEquippedItem(Slot))
                {
                    if (const UItemDataAsset* ItemData = EquippedItem->GetItemData())
                    {
                        for (const FEquipmentGameplayEffect& EffectData : ItemData->EquipmentGameplayEffects)
                        {
                            if (EffectData.GameplayEffectClass)
                            {
                                const UGameplayEffect* GE = EffectData.GameplayEffectClass->GetDefaultObject<UGameplayEffect>();
                                if (!GE) continue;

                                FGameplayEffectSpec Spec(GE, ASC->MakeEffectContext(), EffectData.EffectLevel);
                                for (const FGameplayModifierInfo& Mod : GE->Modifiers)
                                {
                                    float Value = 0.0f;
                                    if (Mod.ModifierMagnitude.AttemptCalculateMagnitude(Spec, Value))
                                    {
                                        // Subtrair o valor do equipamento atual dos stats base
                                        if (Mod.Attribute == URPGAttributeSet::GetAttackAttribute())
                                            BaseStats.Attack -= Value;
                                        else if (Mod.Attribute == URPGAttributeSet::GetArmorAttribute())
                                            BaseStats.Armor -= Value;
                                        else if (Mod.Attribute == URPGAttributeSet::GetMagicDamageAttribute())
                                            BaseStats.MagicDamage -= Value;
                                        else if (Mod.Attribute == URPGAttributeSet::GetAccuracyAttribute())
                                            BaseStats.Accuracy -= Value;
                                        else if (Mod.Attribute == URPGAttributeSet::GetMagicResistanceAttribute())
                                            BaseStats.MagicResist -= Value;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return BaseStats;
}

void UEquipmentComparisonComponent::ApplyCurrentEquipmentEffectsExceptSlot()
{
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;
    
    if (AActor* Owner = GetOwner())
    {
        if (UEquipmentComponent* EquipmentComp = Owner->FindComponentByClass<UEquipmentComponent>())
        {
            for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
            {
                EEquipmentSlot Slot = static_cast<EEquipmentSlot>(SlotIndex);
                
                // Pular o slot que está sendo simulado
                if (ActiveSimulations.Contains(Slot) && ActiveSimulations[Slot])
                    continue;
                
                if (UEquippedItem* EquippedItem = EquipmentComp->GetEquippedItem(Slot))
                {
                    if (const UItemDataAsset* ItemData = EquippedItem->GetItemData())
                    {
                        for (const FEquipmentGameplayEffect& EffectData : ItemData->EquipmentGameplayEffects)
                        {
                            if (EffectData.GameplayEffectClass)
                            {
                                const UGameplayEffect* GE = EffectData.GameplayEffectClass->GetDefaultObject<UGameplayEffect>();
                                if (!GE) continue;

                                FGameplayEffectSpec Spec(GE, ASC->MakeEffectContext(), EffectData.EffectLevel);
                                for (const FGameplayModifierInfo& Mod : GE->Modifiers)
                                {
                                    float Value = 0.0f;
                                    if (Mod.ModifierMagnitude.AttemptCalculateMagnitude(Spec, Value))
                                    {
                                        // Aplicar o valor do equipamento aos stats simulados
                                        if (Mod.Attribute == URPGAttributeSet::GetAttackAttribute())
                                            SimulatedStats.Attack += Value;
                                        else if (Mod.Attribute == URPGAttributeSet::GetArmorAttribute())
                                            SimulatedStats.Armor += Value;
                                        else if (Mod.Attribute == URPGAttributeSet::GetMagicDamageAttribute())
                                            SimulatedStats.MagicDamage += Value;
                                        else if (Mod.Attribute == URPGAttributeSet::GetAccuracyAttribute())
                                            SimulatedStats.Accuracy += Value;
                                        else if (Mod.Attribute == URPGAttributeSet::GetMagicResistanceAttribute())
                                            SimulatedStats.MagicResist += Value;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

bool UEquipmentComparisonComponent::IsItemAlreadyEquipped(const UItemDataAsset* ItemToCheck) const
{
    if (!ItemToCheck) return false;
    
    if (AActor* Owner = GetOwner())
    {
        if (UEquipmentComponent* EquipmentComp = Owner->FindComponentByClass<UEquipmentComponent>())
        {
            // Verificar todos os slots de equipamento
            for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex) // Assumindo 5 slots: Weapon, Armor, Accessory, Boots, Ring
            {
                EEquipmentSlot Slot = static_cast<EEquipmentSlot>(SlotIndex);
                
                if (UEquippedItem* EquippedItem = EquipmentComp->GetEquippedItem(Slot))
                {
                    if (EquippedItem->GetItemData() && 
                        EquippedItem->GetItemData()->ItemName.ToString() == ItemToCheck->ItemName.ToString())
                    {
                        return true; // Item já está equipado
                    }
                }
            }
        }
    }
    
    return false; // Item não está equipado
}

URPGAbilitySystemComponent* UEquipmentComparisonComponent::GetAbilitySystemComponent() const
{
    if (AActor* Owner = GetOwner())
        return Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner));
    return nullptr;
}

float UEquipmentComparisonComponent::GetCurrentAttributeValue(FGameplayAttribute Attribute) const
{
    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    return ASC ? ASC->GetNumericAttribute(Attribute) : 0.f;
}



FString UEquipmentComparisonComponent::GetSmartComparisonText(float Current, float Simulated, const FString& StatName) const
{
    float Diff = Simulated - Current;
    if (FMath::IsNearlyEqual(Current, Simulated, 0.1f))
        return FString::Printf(TEXT("%s: %d"), *StatName, FMath::FloorToInt(Current));
    if (Diff > 0)
        return FString::Printf(TEXT("%s: %d ↑"), *StatName, FMath::FloorToInt(Simulated));
    return FString::Printf(TEXT("%s: %d ↓"), *StatName, FMath::FloorToInt(Simulated));
}

FString UEquipmentComparisonComponent::FormatComparisonText(float Current, float Simulated, const FString& Label) const
{
    return GetSmartComparisonText(Current, Simulated, Label);
}

bool UEquipmentComparisonComponent::IsItemCompatibleWithSlot(const FInventoryItem& Item, EEquipmentSlot Slot) const
{
    if (!Item.IsValid() || !Item.ItemData) return false;

    switch (Slot)
    {
        case EEquipmentSlot::Weapon: return Item.ItemData->ItemCategory == EItemCategory::Weapon;
        case EEquipmentSlot::Armor: return Item.ItemData->ItemCategory == EItemCategory::Armor;
        case EEquipmentSlot::Accessory: return Item.ItemData->ItemCategory == EItemCategory::Accessory;
        case EEquipmentSlot::Boots: return Item.ItemData->ItemCategory == EItemCategory::Boots;
        case EEquipmentSlot::Ring: return Item.ItemData->ItemCategory == EItemCategory::Ring;
        default: return false;
    }
}

bool UEquipmentComparisonComponent::IsItemCurrentlyEquipped(EEquipmentSlot Slot, const FInventoryItem& Item) const
{
    if (!Item.IsValid() || !Item.ItemData) return false;

    if (AActor* Owner = GetOwner())
    {
        if (UEquipmentComponent* EquipmentComp = Owner->FindComponentByClass<UEquipmentComponent>())
        {
            if (UEquippedItem* EquippedItem = EquipmentComp->GetEquippedItem(Slot))
                return EquippedItem->GetItemData() &&
                       EquippedItem->GetItemData()->ItemName.ToString() == Item.ItemData->ItemName.ToString();
        }
    }
    return false;
}

void UEquipmentComparisonComponent::SimulateItemGameplayEffects(const UItemDataAsset* ItemData)
{
    if (!ItemData) return;

    URPGAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (!ASC) return;

    for (const FEquipmentGameplayEffect& EffectData : ItemData->EquipmentGameplayEffects)
    {
        if (EffectData.GameplayEffectClass)
            SimulateGameplayEffect(EffectData.GameplayEffectClass, EffectData.EffectLevel, ASC);
    }
}

void UEquipmentComparisonComponent::SimulateGameplayEffect(TSubclassOf<UGameplayEffect> EffectClass, int32 Level, URPGAbilitySystemComponent* ASC)
{
    if (!EffectClass || !ASC) return;

    const UGameplayEffect* EffectCDO = EffectClass->GetDefaultObject<UGameplayEffect>();
    if (!EffectCDO) return;

    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, Level, ASC->MakeEffectContext());
    if (!SpecHandle.IsValid()) return;

    FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
    if (!Spec) return;

    for (const FGameplayModifierInfo& Modifier : EffectCDO->Modifiers)
    {
        if (!Modifier.Attribute.IsValid()) continue;

        float Value = CalculateModifierValueFromSpec(Modifier, Spec, Level, ASC);
        ApplySimulatedModifier(Modifier.Attribute, Value, Modifier.ModifierOp);
    }
}

float UEquipmentComparisonComponent::CalculateModifierValueFromSpec(const FGameplayModifierInfo& Modifier, const FGameplayEffectSpec* Spec, int32 Level, URPGAbilitySystemComponent* ASC)
{
    if (!Spec) return 0.f;

    float Value = 0.f;
    if (Modifier.ModifierMagnitude.AttemptCalculateMagnitude(*Spec, Value))
        return Value;

    return 0.f;
}

void UEquipmentComparisonComponent::ApplySimulatedModifier(FGameplayAttribute Attribute, float Value, EGameplayModOp::Type Operation)
{
    if (Attribute == URPGAttributeSet::GetAttackAttribute())
        ApplyModifierToStat(SimulatedAttack, CachedCurrentAttack, Value, Operation, TEXT("Ataque"));
    else if (Attribute == URPGAttributeSet::GetArmorAttribute())
        ApplyModifierToStat(SimulatedArmor, CachedCurrentArmor, Value, Operation, TEXT("Armadura"));
    else if (Attribute == URPGAttributeSet::GetMagicDamageAttribute())
        ApplyModifierToStat(SimulatedMagicDamage, CachedCurrentMagicDamage, Value, Operation, TEXT("Dano Mágico"));
    else if (Attribute == URPGAttributeSet::GetAccuracyAttribute())
        ApplyModifierToStat(SimulatedAccuracy, CachedCurrentAccuracy, Value, Operation, TEXT("Precisão"));
    else if (Attribute == URPGAttributeSet::GetMagicResistanceAttribute())
        ApplyModifierToStat(SimulatedMagicResistance, CachedCurrentMagicResistance, Value, Operation, TEXT("Resistência Mágica"));
}

void UEquipmentComparisonComponent::ApplyModifierToStat(float& Stat, float Original, float Value, EGameplayModOp::Type Operation, const FString& StatName)
{
    switch (Operation)
    {
        case EGameplayModOp::Additive: Stat += Value; break;
        case EGameplayModOp::Override: Stat = Value; break;
        default: Stat += Value; break;
    }
}

FString UEquipmentComparisonComponent::GetComparisonTextForStat(float Current, float Simulated) const
{
    if (!HasAnySimulation())
    {
        return FString::Printf(TEXT("%.1f"), Current);
    }

    float Delta = Simulated - Current;
    if (FMath::IsNearlyZero(Delta, 0.01f))
    {
        return FString::Printf(TEXT("%.1f"), Current); // Valor neutro
    }

    // Formato simplificado: apenas valor simulado com seta
    if (Delta > 0)
    {
        return FString::Printf(TEXT("%.1f ↑"), Simulated);
    }
    else
    {
        return FString::Printf(TEXT("%.1f ↓"), Simulated);
    }
}


