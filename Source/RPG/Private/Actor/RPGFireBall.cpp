// Copyright Druid Mechanics

#include "Actor/RPGFireBall.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Core/RPGAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"
#include "RPGAbilityTypes.h"
#include "AbilitySystemComponent.h"

void ARPGFireBall::BeginPlay()
{
	Super::BeginPlay();
	StartOutgoingTimeline();
}

void ARPGFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidOverlap(OtherActor)) return;

	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			const FVector DeathImpulse = GetActorForwardVector() * ExplosionDamageParams.DeathImpulseMagnitude;
			ExplosionDamageParams.DeathImpulse = DeathImpulse;
			
			ExplosionDamageParams.TargetAbilitySystemComponent = TargetASC;
			URPGAbilitySystemLibrary::ApplyDamageEffect(ExplosionDamageParams);
		}
	}
}

void ARPGFireBall::OnHit()
{
	if (GetOwner())
	{
		// TODO: Implementar GameplayCue para efeito visual de explosão
		// FGameplayCueParameters CueParams;
		// CueParams.Location = GetActorLocation();
		// UGameplayCueManager::ExecuteGameplayCue_NonReplicated(GetOwner(), FRPGGameplayTags::Get().GameplayCue_FireBlast, CueParams);
	}
	
	// Chamar a função da classe base para lidar com sons e efeitos
	Super::OnHit();
}

bool ARPGFireBall::IsValidOverlap(AActor* OtherActor)
{
	if (ExplosionDamageParams.SourceAbilitySystemComponent == nullptr) return false;
	AActor* SourceAvatarActor = ExplosionDamageParams.SourceAbilitySystemComponent->GetAvatarActor();
	if (SourceAvatarActor == OtherActor) return false;
	if (!URPGAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return false;

	return true;
} 