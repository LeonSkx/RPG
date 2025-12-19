#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameplayTagContainer.h"
#include "Inventory/Core/InventoryEnums.h"
#include "Character/PlayerClassInfo.h"
#include "ItemDataAsset.generated.h"

// =========================
// Forward Declarations
// =========================
class ABaseItem;
class UGameplayEffect;
class UGameplayAbility;
class URPGGameplayAbility;

// =========================
// Structs auxiliares
// =========================

// Efeitos GAS aplicados ao equipar
USTRUCT(BlueprintType)
struct FEquipmentGameplayEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
    TSubclassOf<UGameplayEffect> GameplayEffectClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "1"))
    int32 EffectLevel = 1;
};

// Abilities GAS aplicadas ao equipar
USTRUCT(BlueprintType)
struct FEquipmentGameplayAbility
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    TSubclassOf<URPGGameplayAbility> GameplayAbilityClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities", meta = (ClampMin = "1"))
    int32 AbilityLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    bool bActivateOnEquip = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
    FGameplayTag InputTag;
};

// =========================
// Classe Principal
// =========================

/**
 * Data Asset que define um item do jogo
 */
UCLASS(BlueprintType)
class RPG_API UItemDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UItemDataAsset();

    // === DADOS BÁSICOS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item", meta = (MultiLine = true))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    EItemCategory ItemCategory = EItemCategory::None;

    // ID único (usado em quests, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FString ItemID = "None_Item";

    // === SUBTIPOS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Subtypes",
              meta = (EditCondition = "ItemCategory==EItemCategory::Consumable", EditConditionHides))
    EConsumableType ConsumableType = EConsumableType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Subtypes",
              meta = (EditCondition = "ItemCategory==EItemCategory::Weapon", EditConditionHides))
    EWeaponType WeaponType = EWeaponType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Subtypes",
              meta = (EditCondition = "ItemCategory==EItemCategory::Accessory", EditConditionHides))
    EAccessoryType AccessoryType = EAccessoryType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Subtypes",
              meta = (EditCondition = "ItemCategory==EItemCategory::Ring", EditConditionHides))
    ERingType RingType = ERingType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Subtypes",
              meta = (EditCondition = "ItemCategory==EItemCategory::Armor", EditConditionHides))
    EArmorType ArmorType = EArmorType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Subtypes",
              meta = (EditCondition = "ItemCategory==EItemCategory::Boots", EditConditionHides))
    EBootsType BootsType = EBootsType::None;

    // === VISUAIS ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<UTexture2D> TypeIcon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visual", meta = (ContentDir))
    FString ItemMeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

    // === SOCKETS (Visual + Combat) ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sockets",
             meta = (EditCondition = "ItemCategory==EItemCategory::Weapon || ItemCategory==EItemCategory::Armor || ItemCategory==EItemCategory::Accessory || ItemCategory==EItemCategory::Ring", EditConditionHides))
    bool bUseSecondarySocketVisual = false;

    // Posição visual de armas/equipamentos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets",
             meta = (EditCondition = "ItemCategory==EItemCategory::Weapon || ItemCategory==EItemCategory::Armor || ItemCategory==EItemCategory::Accessory || ItemCategory==EItemCategory::Ring || ItemCategory==EItemCategory::Boots", EditConditionHides))
    TArray<FName> EquipmentSockets;

    // Posição de spawn para efeitos/dano
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sockets",
             meta = (EditCondition = "ItemCategory==EItemCategory::Weapon", EditConditionHides))
    TArray<FName> DamageSockets;

    // === PROPRIEDADES ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties", meta = (ClampMin = "1"))
    int32 RequiredLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
    bool bIsStackable = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties",
             meta = (EditCondition = "bIsStackable", EditConditionHides, ClampMin = "1"))
    int32 MaxStackSize = 99;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
    bool bCanBeDiscarded = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
    bool bIsRare = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Properties")
    bool bIsPassive = false;

    // === RESTRIÇÕES POR CLASSE ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Class Restrictions")
    TArray<EPlayerClass> AllowedClasses;

    UFUNCTION(BlueprintPure, Category = "Class Restrictions")
    bool CanClassUse(EPlayerClass PlayerClass) const;

    // === GAS: EFEITOS AO EQUIPAR ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|GAS Effects",
             meta = (EditCondition = "ItemCategory==EItemCategory::Weapon || ItemCategory==EItemCategory::Armor || ItemCategory==EItemCategory::Accessory || ItemCategory==EItemCategory::Ring || ItemCategory==EItemCategory::Boots", EditConditionHides))
    TArray<FEquipmentGameplayEffect> EquipmentGameplayEffects;

    // === GAS: ABILITIES AO EQUIPAR ===
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment|GAS Abilities",
             meta = (EditCondition = "ItemCategory==EItemCategory::Weapon || ItemCategory==EItemCategory::Armor || ItemCategory==EItemCategory::Accessory || ItemCategory==EItemCategory::Ring || ItemCategory==EItemCategory::Boots", EditConditionHides))
    TArray<FEquipmentGameplayAbility> EquipmentGameplayAbilities;
};
