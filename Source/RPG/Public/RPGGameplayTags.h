// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * RPGGameplayTags
 *
 * Singleton containing native Gameplay Tags
 */

struct FRPGGameplayTags
{
public:
    static const FRPGGameplayTags& Get() { return GameplayTags;}
    static void InitializeNativeGameplayTags();

	// Primary Attributes
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intelligence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;

	// Secondary Attributes
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_Attack;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CriticalHitChance;
	FGameplayTag Attributes_Secondary_CriticalHitDamage;
	FGameplayTag Attributes_Secondary_CriticalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_MagicDamage;
	FGameplayTag Attributes_Secondary_MagicResistance;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;
	FGameplayTag Attributes_Secondary_MaxEnergy;
	
	// Meta Attributes
	FGameplayTag Attributes_Meta_IncomingXP;

	// Input Tags
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;

	// Menu Input Tags
	FGameplayTag InputTag_OpenMenu;
	FGameplayTag InputTag_CloseMenu;
	FGameplayTag InputTag_ToggleMenu;

	// Party System Input Tags
	FGameplayTag InputTag_NextPartyMember;
	FGameplayTag InputTag_PreviousPartyMember;
	FGameplayTag InputTag_PartyMember_1;
	FGameplayTag InputTag_PartyMember_2;
	FGameplayTag InputTag_PartyMember_3;
	FGameplayTag InputTag_PartyMember_4;

	// Player Block Tags
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;

	// Abilities Tags
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_HitReact;
	FGameplayTag Abilities_ActivateOnGiven;

	// Combo System Tags
	FGameplayTag Ability_Combo_Change;
	FGameplayTag Ability_Combo_Change_End;

	// Damage Tags
	FGameplayTag Damage;
	FGameplayTag Damage_Physical;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Arcane;

	// Debuff Tags
	FGameplayTag Debuff_Stun;

	// Effects Tags
	FGameplayTag Effects_HitReact;

	// Combat Sockets
	FGameplayTag CombatSocket_Weapon;

	// Montage Tags
	FGameplayTag Montage_Attack_Weapon;
	FGameplayTag Montage_Attack_RightHand;
	FGameplayTag Montage_Attack_LeftHand;

private:
    static FRPGGameplayTags GameplayTags;
};
