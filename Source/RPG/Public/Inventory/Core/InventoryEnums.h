#pragma once

#include "CoreMinimal.h"
#include "InventoryEnums.generated.h"

// Removido: EItemType substituído por categorias específicas em EItemCategory

// Subtipos por categoria/tipo

UENUM(BlueprintType)
enum class EConsumableType : uint8
{
    None    UMETA(DisplayName = "None"),
    Gum     UMETA(DisplayName = "Gum"),
    Bottle  UMETA(DisplayName = "Bottle"),
    Herb    UMETA(DisplayName = "Herb")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None     UMETA(DisplayName = "None"),
    Melee    UMETA(DisplayName = "Melee"),
    Ranged   UMETA(DisplayName = "Ranged"),
    Blade    UMETA(DisplayName = "Blade"),
    Impact   UMETA(DisplayName = "Impact")
};

UENUM(BlueprintType)
enum class EAccessoryType : uint8
{
    None     UMETA(DisplayName = "None"),
    Necklace UMETA(DisplayName = "Necklace"),
    Earring  UMETA(DisplayName = "Earring")
};

UENUM(BlueprintType)
enum class EArmorType : uint8
{
    None   UMETA(DisplayName = "None"),
    Light  UMETA(DisplayName = "Light"),
    Heavy  UMETA(DisplayName = "Heavy")
};

UENUM(BlueprintType)
enum class EBootsType : uint8
{
    None   UMETA(DisplayName = "None"),
    Light  UMETA(DisplayName = "Light"),
    Heavy  UMETA(DisplayName = "Heavy")
};

UENUM(BlueprintType)
enum class ERingType : uint8
{
    None        UMETA(DisplayName = "None"),
    Combat      UMETA(DisplayName = "Combat"),
    Magic       UMETA(DisplayName = "Magic"),
    Defense     UMETA(DisplayName = "Defense"),
    Utility     UMETA(DisplayName = "Utility"),
    Healing     UMETA(DisplayName = "Healing"),
    Elemental   UMETA(DisplayName = "Elemental")
};

/**
 * Categorias principais do inventário
 * Organizadas por hierarquia lógica e funcionalidade
 * DisplayName será usado para localização futura
 */
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    None        UMETA(DisplayName = "None"),
    
    // === CATEGORIA ESPECIAL ===
    New         UMETA(DisplayName = "New"),
    
    // === EQUIPAMENTOS DE COMBATE ===
    Weapon      UMETA(DisplayName = "Weapon"),
    
    // === EQUIPAMENTOS DEFENSIVOS ===
    Armor       UMETA(DisplayName = "Armor"),
    Boots       UMETA(DisplayName = "Boots"),
    
    // === ACESSÓRIOS ===
    Accessory   UMETA(DisplayName = "Accessory"),
    Ring        UMETA(DisplayName = "Ring"),
    
    // === CONSUMÍVEIS E UTILITÁRIOS ===
    Consumable  UMETA(DisplayName = "Consumable"),
    Material    UMETA(DisplayName = "Material"),
    
    // === CATEGORIAS ESPECIAIS ===
    Valuable    UMETA(DisplayName = "Valuable"),
    Cosmetic    UMETA(DisplayName = "Cosmetic"),
    Expansion   UMETA(DisplayName = "Expansion")
};

/**
 * Filtros principais de UI para o inventário
 * Sistema simplificado que usa os enums de subtipo diretamente
 * DisplayName será usado para localização futura
 */
UENUM(BlueprintType)
enum class EInventoryFilterCategory : uint8
{
    None        UMETA(DisplayName = "None"),
    NewItems    UMETA(DisplayName = "NewItems"),
    
    // === CATEGORIAS PRINCIPAIS ===
    Consumable  UMETA(DisplayName = "Consumable"),
    Weapon      UMETA(DisplayName = "Weapon"),
    Accessory   UMETA(DisplayName = "Accessory"),
    Ring        UMETA(DisplayName = "Ring"),
    Armor       UMETA(DisplayName = "Armor"),
    Boots       UMETA(DisplayName = "Boots"),
    
    // === CATEGORIAS ESPECIAIS ===
    Materials   UMETA(DisplayName = "Materials"),
    Passive     UMETA(DisplayName = "Passive"),
    Quest       UMETA(DisplayName = "Quest"),
    Cosmetic    UMETA(DisplayName = "Cosmetic"),
    Expansion   UMETA(DisplayName = "Expansion"),
    
    // === FILTROS COMPOSTOS (para casos especiais) ===
    AllEquipment UMETA(DisplayName = "AllEquipment"),
    AllWeapons   UMETA(DisplayName = "AllWeapons"),
    AllArmor     UMETA(DisplayName = "AllArmor")
};

/**
 * Tipos de atributos que podem ser modificados por itens
 */
UENUM(BlueprintType)
enum class EAttributeType : uint8
{
    None            UMETA(DisplayName = "Nenhum"),
    Health          UMETA(DisplayName = "Saúde"),
    Mana            UMETA(DisplayName = "Mana"),
    Stamina         UMETA(DisplayName = "Stamina"),
    Strength        UMETA(DisplayName = "Força"),
    Dexterity       UMETA(DisplayName = "Destreza"),
    Intelligence    UMETA(DisplayName = "Inteligência"),
    Vitality        UMETA(DisplayName = "Vitalidade"),
    PhysicalDamage  UMETA(DisplayName = "Dano Físico"),
    MagicDamage     UMETA(DisplayName = "Dano Mágico"),
    PhysicalDefense UMETA(DisplayName = "Defesa Física"),
    MagicDefense    UMETA(DisplayName = "Defesa Mágica"),
    CriticalChance  UMETA(DisplayName = "Chance Crítica"),
    CriticalDamage  UMETA(DisplayName = "Dano Crítico"),
    MovementSpeed   UMETA(DisplayName = "Velocidade"),
    AttackSpeed     UMETA(DisplayName = "Velocidade de Ataque"),
    HealthRegen     UMETA(DisplayName = "Regeneração de Saúde"),
    ManaRegen       UMETA(DisplayName = "Regeneração de Mana"),
    StaminaRegen    UMETA(DisplayName = "Regeneração de Stamina")
}; 
