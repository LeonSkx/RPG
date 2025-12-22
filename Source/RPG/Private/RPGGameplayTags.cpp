// Copyright Druid Mechanics


#include "RPGGameplayTags.h"
#include "GameplayTagsManager.h"

FRPGGameplayTags FRPGGameplayTags::GameplayTags;

void FRPGGameplayTags::InitializeNativeGameplayTags()
{
	/*
	 * Primary Attributes
	 */
	GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Strength"),
		FString("Increases physical damage")
		);

	GameplayTags.Attributes_Primary_Intelligence = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Intelligence"),
		FString("Increases magical damage")
		);

	GameplayTags.Attributes_Primary_Resilience = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Resilience"),
		FString("Increases Armor and Armor Penetration")
		);

	GameplayTags.Attributes_Primary_Vigor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Primary.Vigor"),
		FString("Increases Health")
		);

	/*
	 * Secondary Attributes
	 */
	
	GameplayTags.Attributes_Secondary_Armor = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Armor"),
		FString("Reduces damage taken, improves Block Chance")
		);

	GameplayTags.Attributes_Secondary_ArmorPenetration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ArmorPenetration"),
		FString("Ignores Percentage of enemy Armor, increases Critical Hit Chance")
		);

	GameplayTags.Attributes_Secondary_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.Attack"),
		FString("Base physical attack power from character and equipment")
		);

	GameplayTags.Attributes_Secondary_BlockChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.BlockChance"),
		FString("Chance to cut incoming damage in half")
		);

	GameplayTags.Attributes_Secondary_CriticalHitChance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitChance"),
		FString("Chance to double damage plus critical hit bonus")
		);

	GameplayTags.Attributes_Secondary_CriticalHitDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitDamage"),
		FString("Bonus damage added when a critical hit is scored")
		);

	GameplayTags.Attributes_Secondary_CriticalHitResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitResistance"),
		FString("Reduces Critical Hit Chance of attacking enemies")
		);

	GameplayTags.Attributes_Secondary_HealthRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.HealthRegeneration"),
		FString("Amount of Health regenerated every 1 second")
		);

	GameplayTags.Attributes_Secondary_MagicDamage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MagicDamage"),
		FString("Base magical damage power from character and equipment")
		);

	GameplayTags.Attributes_Secondary_MagicResistance = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MagicResistance"),
		FString("Reduces magical damage taken")
		);

	GameplayTags.Attributes_Secondary_ManaRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.ManaRegeneration"),
		FString("Amount of Mana regenerated every 1 second")
		);

	GameplayTags.Attributes_Secondary_MaxHealth = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxHealth"),
		FString("Maximum amount of Health obtainable")
		);

	GameplayTags.Attributes_Secondary_MaxMana = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxMana"),
		FString("Maximum amount of Mana obtainable")
		);

	GameplayTags.Attributes_Secondary_MaxEnergy = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxEnergy"),
		FString("Maximum amount of Energy obtainable")
		);

	/*
	 * Meta Attributes
	 */

	GameplayTags.Attributes_Meta_IncomingXP = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Attributes.Meta.IncomingXP"),
		FString("Incoming XP Meta Attribute")
		);

	/*
	 * Input Tags
	 */
	
	GameplayTags.InputTag_LMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.LMB"),
		FString("Input Tag for Left Mouse Button")
		);

	GameplayTags.InputTag_RMB = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.RMB"),
		FString("Input Tag for Right Mouse Button")
		);

	/*
	 * Menu Input Tags
	 */

	GameplayTags.InputTag_OpenMenu = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.OpenMenu"),
		FString("Open Menu Input Tag")
		);

	GameplayTags.InputTag_CloseMenu = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.CloseMenu"),
		FString("Close Menu Input Tag")
		);

	GameplayTags.InputTag_ToggleMenu = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.ToggleMenu"),
		FString("Toggle Menu Input Tag")
		);

	/*
	 * Party System Input Tags
	 */

	GameplayTags.InputTag_NextPartyMember = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.NextPartyMember"),
		FString("Input Tag for switching to next party member")
		);

	GameplayTags.InputTag_PreviousPartyMember = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.PreviousPartyMember"),
		FString("Input Tag for switching to previous party member")
		);

	GameplayTags.InputTag_PartyMember_1 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.PartyMember.1"),
		FString("Input Tag for switching directly to party member 1")
		);

	GameplayTags.InputTag_PartyMember_2 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.PartyMember.2"),
		FString("Input Tag for switching directly to party member 2")
		);

	GameplayTags.InputTag_PartyMember_3 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.PartyMember.3"),
		FString("Input Tag for switching directly to party member 3")
		);

	GameplayTags.InputTag_PartyMember_4 = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("InputTag.PartyMember.4"),
		FString("Input Tag for switching directly to party member 4")
		);

	/*
	 * Player Block Tags
	 */

	GameplayTags.Player_Block_InputPressed = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputPressed"),
		FString("Block Input Pressed callback for input")
		);

	GameplayTags.Player_Block_InputHeld = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputHeld"),
		FString("Block Input Held callback for input")
		);

	GameplayTags.Player_Block_InputReleased = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Player.Block.InputReleased"),
		FString("Block Input Released callback for input")
		);

	/*
	 * Abilities Tags
	 */

	GameplayTags.Abilities_Attack = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.Attack"),
		FString("Attack Ability Tag")
		);

	GameplayTags.Abilities_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.HitReact"),
		FString("Hit React Ability")
		);

	GameplayTags.Abilities_ActivateOnGiven = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Abilities.ActivateOnGiven"),
		FString("Ability activates automatically when given to the character")
		);

	/*
	 * Combo System Tags
	 */

	GameplayTags.Ability_Combo_Change = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Combo.Change"),
		FString("Base tag for combo change events. Use Ability.Combo.Change.{SectionName} to specify next combo section"));

	GameplayTags.Ability_Combo_Change_End = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Ability.Combo.Change.End"),
		FString("Tag sent when combo window closes, preventing further combo continuation"));

	/*
	 * Combat Sockets
	 */

	GameplayTags.CombatSocket_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("CombatSocket.Weapon"),
		FString("Weapon")
		);

	/*
	 * Montage Tags
	 */

	GameplayTags.Montage_Attack_Weapon = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.Weapon"),
		FString("Attack with Weapon")
		);

	GameplayTags.Montage_Attack_RightHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.RightHand"),
		FString("Attack with Right Hand")
		);

	GameplayTags.Montage_Attack_LeftHand = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Montage.Attack.LeftHand"),
		FString("Attack with Left Hand")
		);

	/*
	 * Damage Tags
	 */

	GameplayTags.Damage = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage"),
		FString("Damage")
		);

	GameplayTags.Damage_Physical = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Physical"),
		FString("Physical Damage Type")
		);

	GameplayTags.Damage_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Fire"),
		FString("Fire Damage Type")
		);

	GameplayTags.Damage_Lightning = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Lightning"),
		FString("Lightning Damage Type")
		);

	GameplayTags.Damage_Arcane = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Damage.Arcane"),
		FString("Arcane Damage Type")
		);

	/*
	 * Debuff Tags
	 */

	GameplayTags.Debuff_Stun = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Debuff.Stun"),
		FString("Debuff for Lightning damage")
		);

	/*
	 * Effects Tags
	 */

	GameplayTags.Effects_HitReact = UGameplayTagsManager::Get().AddNativeGameplayTag(
		FName("Effects.HitReact"),
		FString("Tag granted when Hit Reacting")
		);
}
