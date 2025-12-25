// Copyright (c) 2025 RPG Yumi Project. All rights reserved.

#include "AbilitySystem/Abilities/GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "RPGGameplayTags.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

UGA_Combo::UGA_Combo()
{
	SetAssetTags(FGameplayTagContainer(FRPGGameplayTags::Get().Abilities_Attack));
	BlockAbilitiesWithTag.AddTag(FRPGGameplayTags::Get().Abilities_Attack);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		if (!ComboMontage)
		{
			K2_EndAbility();
			return;
		}

		UAbilityTask_PlayMontageAndWait* PlayComboMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ComboMontage);
		PlayComboMontageTask->OnBlendOut.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCancelled.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnCompleted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->OnInterrupted.AddDynamic(this, &UGA_Combo::K2_EndAbility);
		PlayComboMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetComboChangedEventTag(), nullptr, false, false);
		WaitComboChangeEventTask->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangedEventReceived);
		WaitComboChangeEventTask->ReadyForActivation();
	}

	NextComboName = NAME_None;
	
	// Escutar evento de input pressionado para fazer o combo
	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		SetupWaitComboInputPress();
	}
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	return FRPGGameplayTags::Get().Ability_Combo_Change;
}

FGameplayTag UGA_Combo::GetComboChangedEventEndTag()
{
	return FRPGGameplayTags::Get().Ability_Combo_Change_End;
}

void UGA_Combo::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Limpar a task de input antes de terminar a ability
	if (IsValid(WaitInputPressedTask))
	{
		WaitInputPressedTask->EndTask();
		WaitInputPressedTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Combo::SetupWaitComboInputPress()
{
	// Cancelar a task anterior se existir
	if (IsValid(WaitInputPressedTask))
	{
		WaitInputPressedTask->EndTask();
		WaitInputPressedTask = nullptr;
	}

	// Escutar evento de input pressionado ao invés de WaitInputPress
	WaitInputPressedTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FRPGGameplayTags::Get().Events_Abilities_InputPressed, nullptr, false, false);
	WaitInputPressedTask->EventReceived.AddDynamic(this, &UGA_Combo::HandleInputPress);
	WaitInputPressedTask->ReadyForActivation();
}

void UGA_Combo::HandleInputPress(FGameplayEventData EventData)
{
	// Verificar se o input pressionado corresponde ao StartupInputTag desta ability
	// O InputTag foi adicionado no InstigatorTags do Payload
	// Verificar diretamente se o InstigatorTags contém o StartupInputTag
	if (EventData.InstigatorTags.HasTagExact(StartupInputTag))
	{
		// Reconfigurar o listener para o próximo input
		SetupWaitComboInputPress();
		// Tentar commitar o combo
		TryCommitCombo();
	}
	else
	{
		// Se não for o input correto, continuar escutando
		SetupWaitComboInputPress();
	}
}

void UGA_Combo::TryCommitCombo()
{
	if (NextComboName == NAME_None)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !Character->GetMesh())
	{
		return;
	}

	UAnimInstance* OwnerAnimInst = Character->GetMesh()->GetAnimInstance();
	if (!OwnerAnimInst || !ComboMontage)
	{
		return;
	}

	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(ComboMontage), NextComboName, ComboMontage);
}

void UGA_Combo::ComboChangedEventReceived(FGameplayEventData Data)
{
	FGameplayTag EventTag = Data.EventTag;

	if (EventTag == GetComboChangedEventEndTag())
	{
		NextComboName = NAME_None;
		return;
	}
	
	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);

	NextComboName = TagNames.Last();
}
