#include "AbilitySystem/Abilities/Magic/Lazer/GA_Lazer.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitCancel.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Components/SkeletalMeshComponent.h"


void UGA_Lazer::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!K2_CommitAbility() || !LazerMontage)
	{
		K2_EndAbility();
		return;
	}

	if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		UAbilityTask_PlayMontageAndWait* PlayerLazerMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, LazerMontage);
		PlayerLazerMontageTask->OnBlendOut.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		PlayerLazerMontageTask->OnCancelled.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		PlayerLazerMontageTask->OnInterrupted.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		PlayerLazerMontageTask->OnCompleted.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		PlayerLazerMontageTask->ReadyForActivation();

		UAbilityTask_WaitGameplayEvent* WaitShootEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetShootTag());
		WaitShootEvent->EventReceived.AddDynamic(this, &UGA_Lazer::ShootLazer);
		WaitShootEvent->ReadyForActivation();

		UAbilityTask_WaitCancel* WaitCanel = UAbilityTask_WaitCancel::WaitCancel(this);
		WaitCanel->OnCancel.AddDynamic(this, &UGA_Lazer::K2_EndAbility);
		WaitCanel->ReadyForActivation();
	}
}
void UGA_Lazer::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* OwnerAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (OwnerAbilitySystemComponent)
	{
		// On Going Consumption Effect - Remove o efeito de consumo contínuo quando a habilidade termina
		/*if (OnGoingConsumtionEffectHandle.IsValid())
		{
			OwnerAbilitySystemComponent->RemoveActiveGameplayEffect(OnGoingConsumtionEffectHandle);
			OnGoingConsumtionEffectHandle = FActiveGameplayEffectHandle();
		}*/

		// On Going Consumption Effect - Remove o delegate que monitora mudanças de mana
		/*if (ManaChangeDelegateHandle.IsValid())
		{
			OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetManaAttribute()).Remove(ManaChangeDelegateHandle);
			ManaChangeDelegateHandle.Reset();
		}*/
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FGameplayTag UGA_Lazer::GetShootTag()
{
    return FGameplayTag::RequestGameplayTag(TEXT("Ability.Lazer.Shoot"));
}

void UGA_Lazer::ShootLazer(FGameplayEventData Payload)
{
	if (!LazerTargetActorClass)
	{
		K2_EndAbility();
		return;
	}

	if (K2_HasAuthority())
	{
		// On Going Consumption Effect - Aplica o efeito de consumo contínuo de mana
		/*if (OnGoingConsumtionEffect)
		{
			OnGoingConsumtionEffectHandle = BP_ApplyGameplayEffectToOwner(OnGoingConsumtionEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}*/
		// On Going Consumption Effect - Configura o delegate para monitorar mudanças de mana e encerrar a habilidade quando a mana acabar
		UAbilitySystemComponent* OwnerAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
		/*if (OwnerAbilitySystemComponent && !ManaChangeDelegateHandle.IsValid() && OnGoingConsumtionEffect)
		{
			ManaChangeDelegateHandle = OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetManaAttribute()).AddUObject(this, &UGA_Lazer::ManaUpdated);
		}*/
	}

	UAbilityTask_WaitTargetData* WaitDamageTargetTask = UAbilityTask_WaitTargetData::WaitTargetData(this, NAME_None, EGameplayTargetingConfirmation::CustomMulti, LazerTargetActorClass);
	if (!WaitDamageTargetTask)
	{
		return;
	}

	WaitDamageTargetTask->ValidData.AddDynamic(this, &UGA_Lazer::TargetReceived);
	WaitDamageTargetTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor = nullptr;
	WaitDamageTargetTask->BeginSpawningActor(this, LazerTargetActorClass, TargetActor);
	ATargetActor_Line* LineTargetActor = Cast<ATargetActor_Line>(TargetActor);
	if (LineTargetActor)
	{
		LineTargetActor->ConfigureTargetSetting(TargetRange, DetectionCylinderRadius, TargetingInterval, GetOwnerTeamId(), ShouldDrawDebug());
	}

	WaitDamageTargetTask->FinishSpawningActor(this, TargetActor);

	if (LineTargetActor)
	{
		if (USkeletalMeshComponent* OwningMesh = GetOwningComponentFromActorInfo())
		{
			LineTargetActor->AttachToComponent(OwningMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetActorAttachSocketName);
		}
	}
}

// On Going Consumption Effect - Função chamada quando a mana é atualizada, verifica se ainda é possível aplicar o efeito de consumo
// Se não for mais possível (ex: mana insuficiente), encerra a habilidade
/*void UGA_Lazer::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	UAbilitySystemComponent* OwnerAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (OwnerAbilitySystemComponent && OnGoingConsumtionEffect && !OwnerAbilitySystemComponent->CanApplyAttributeModifiers(OnGoingConsumtionEffect.GetDefaultObject(), GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo), MakeEffectContext(CurrentSpecHandle, CurrentActorInfo)))
	{
		K2_EndAbility();
	}
}*/

void UGA_Lazer::TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	if (TargetDataHandle.IsValid(0))
	{
		if (K2_HasAuthority() && HitDamageEffect)
		{
			BP_ApplyGameplayEffectToTarget(TargetDataHandle, HitDamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}

		if (AActor* AvatarActor = GetAvatarActorFromActorInfo())
		{
			PushTargets(TargetDataHandle, AvatarActor->GetActorForwardVector() * HitPushSpeed);
		}
	}
}

