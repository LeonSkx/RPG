// Copyright Druid Mechanics

#include "Actor/RPGProjectile.h"
#include "RPG/RPG.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Hearing.h"
#include "AbilitySystem/Core/RPGAbilitySystemLibrary.h"

ARPGProjectile::ARPGProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void ARPGProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	SetReplicateMovement(true);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ARPGProjectile::OnSphereOverlap);

    // Removed looping sound
}

void ARPGProjectile::Destroyed()
{
    if (!bHit && !HasAuthority()) OnHit();
	Super::Destroyed();
}

void ARPGProjectile::OnHit()
{
    // Visual/Som removidos
	bHit = true;
}

void ARPGProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                     bool bFromSweep, const FHitResult& SweepResult)
{
	if (!DamageEffectSpecHandle.Data.IsValid() || DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser() == OtherActor)
	{
		return;
	}

	// Checagem de aliados: não aplicar dano em amigos
	AActor* EffectCauser = DamageEffectSpecHandle.Data.IsValid() ? DamageEffectSpecHandle.Data.Get()->GetContext().GetEffectCauser() : nullptr;
	if (!URPGAbilitySystemLibrary::IsNotFriend(EffectCauser, OtherActor))
	{
		return;
	}
	
	if (!bHit) OnHit();
	
	// Reportar evento de som para a IA
	// O instigador é o dono do projétil, não o projétil em si.
	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
		UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 1.0f, MyOwner);
	}

	if (HasAuthority())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
		}
		Destroy();
	}
	else bHit = true;
} 