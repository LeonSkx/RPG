#include "Character/RPGCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "RPG/RPG.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Components/SceneComponent.h"
#include "Party/PartySubsystem.h"
#include "Progression/ProgressionSubsystem.h"
#include "Character/RPGCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/EngineTypes.h"
#include "MotionWarpingComponent.h"

ARPGCharacterBase::ARPGCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	TeamID = FGenericTeamId(static_cast<uint8>(Team));

	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
	
	WeaponTipSocketName = FName("WeaponTipSocket");
}

void ARPGCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARPGCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARPGCharacterBase, bIsStunned);
	DOREPLIFETIME(ARPGCharacterBase, bIsBurned);
	DOREPLIFETIME(ARPGCharacterBase, bIsBeingShocked);
}

float ARPGCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnDamageDelegate.Broadcast(DamageTaken);
	return DamageTaken;
}

UAbilitySystemComponent* ARPGCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* ARPGCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void ARPGCharacterBase::Die(const FVector& DeathImpulse)
{
	if (Weapon)
	{
		Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	}
	
	MulticastHandleDeath(DeathImpulse);
}

FOnDeathSignature& ARPGCharacterBase::GetOnDeathDelegate()
{
	return OnDeathDelegate;
}

void ARPGCharacterBase::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	if (bDead)
	{
		return;
	}
	
	// Remover todos os warp targets do Motion Warping ao morrer
	RemoveAllWarpTargets_Implementation();
	
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), GetActorRotation());
	
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Dissolve();
	
	if (Weapon && IsValid(WeaponDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* WeaponMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, WeaponMatInst);
		StartWeaponDissolveTimeline(WeaponMatInst);
	}
	
	bDead = true;
	OnDeathDelegate.Broadcast(this);
}

void ARPGCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f : BaseWalkSpeed;
}

void ARPGCharacterBase::OnRep_Stunned()
{
}

void ARPGCharacterBase::OnRep_Burned()
{
}

void ARPGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetGenericTeamId(FGenericTeamId(static_cast<uint8>(Team)));
}

FVector ARPGCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	if (MontageTag == FRPGGameplayTags::Get().Montage_Attack_Weapon)
	{
		if (Weapon && WeaponTipSocketName != NAME_None && Weapon->DoesSocketExist(WeaponTipSocketName))
		{
			return Weapon->GetSocketLocation(WeaponTipSocketName);
		}
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (RightHandSocketName != NAME_None && MeshComp->DoesSocketExist(RightHandSocketName))
			{
				return MeshComp->GetSocketLocation(RightHandSocketName);
			}
		}
	}
	else if (MontageTag == FRPGGameplayTags::Get().Montage_Attack_RightHand)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (RightHandSocketName != NAME_None && MeshComp->DoesSocketExist(RightHandSocketName))
			{
				return MeshComp->GetSocketLocation(RightHandSocketName);
			}
		}
	}
	else if (MontageTag == FRPGGameplayTags::Get().Montage_Attack_LeftHand)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (LeftHandSocketName != NAME_None && MeshComp->DoesSocketExist(LeftHandSocketName))
			{
				return MeshComp->GetSocketLocation(LeftHandSocketName);
			}
		}
	}
	return GetActorLocation();
}

bool ARPGCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* ARPGCharacterBase::GetAvatar_Implementation()
{
	return this;
}

TArray<FTaggedMontage> ARPGCharacterBase::GetAttackMontages_Implementation()
{
	return AttackMontages;
}

UNiagaraSystem* ARPGCharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

FTaggedMontage ARPGCharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	for (FTaggedMontage TaggedMontage : AttackMontages)
	{
		if (TaggedMontage.MontageTag == MontageTag)
		{
			return TaggedMontage;
		}
	}
	return FTaggedMontage();
}

USkeletalMeshComponent* ARPGCharacterBase::GetWeapon_Implementation()
{
	return Weapon;
}

int32 ARPGCharacterBase::GetMinionCount_Implementation()
{
	return MinionCount;
}

void ARPGCharacterBase::IncremenetMinionCount_Implementation(int32 Amount)
{
	MinionCount += Amount;
}

int32 ARPGCharacterBase::GetCharacterLevel_Implementation() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			if (const ARPGCharacter* RPGCharacter = Cast<ARPGCharacter>(this))
			{
				return ProgressionSubsystem->GetCharacterLevel(RPGCharacter);
			}
		}
	}
	return 1;
}



FOnASCRegistered& ARPGCharacterBase::GetOnASCRegisteredDelegate()
{
	return OnAscRegistered;
}

void ARPGCharacterBase::SetIsBeingShocked_Implementation(bool bInShock)
{
	bIsBeingShocked = bInShock;
}

bool ARPGCharacterBase::IsBeingShocked_Implementation() const
{
	return bIsBeingShocked;
}

FOnDamageSignature& ARPGCharacterBase::GetOnDamageSignature()
{
	return OnDamageDelegate;
}

void ARPGCharacterBase::InitAbilityActorInfo()
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		return;
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

void ARPGCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
    UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponent();
    if (!IsValid(AbilitySystem))
    {
        return;
    }
    if (!GameplayEffectClass)
    {
        return;
    }

    FGameplayEffectContextHandle ContextHandle = AbilitySystem->MakeEffectContext();
    ContextHandle.AddSourceObject(this);
    const FGameplayEffectSpecHandle SpecHandle = AbilitySystem->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
    if (!SpecHandle.IsValid() || SpecHandle.Data.Get() == nullptr)
    {
        return;
    }
    AbilitySystem->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), AbilitySystem);
}

TArray<AActor*> ARPGCharacterBase::GetLivePlayersWithinRadius(double Radius, const FVector& SphereOrigin)
{
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> OutActors;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), SphereOrigin, Radius, ObjectTypes, nullptr, ActorsToIgnore, OutActors);
	
	for (AActor* Actor : OutActors)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Actor))
		{
			if (!CombatInterface->IsDead())
			{
				OverlappingActors.AddUnique(Actor);
			}
		}
	}
	
	return OverlappingActors;
}

void ARPGCharacterBase::InitializeDefaultAttributes() const
{
	if (!AbilitySystemComponent || !IsValid(AbilitySystemComponent))
	{
		return;
	}

	int32 CharacterLevel = 1;
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			if (const ARPGCharacter* RPGCharacter = Cast<ARPGCharacter>(this))
			{
				CharacterLevel = ProgressionSubsystem->GetCharacterLevel(RPGCharacter);
			}
		}
	}

	ApplyEffectToSelf(DefaultPrimaryAttributes, static_cast<float>(CharacterLevel));
	ApplyEffectToSelf(DefaultSecondaryAttributes, static_cast<float>(CharacterLevel));
	ApplyEffectToSelf(DefaultVitalAttributes, static_cast<float>(CharacterLevel));
	ApplyEffectToSelf(DefaultCombatAttributes, static_cast<float>(CharacterLevel));
}

void ARPGCharacterBase::AddCharacterAbilities()
{
	if (!AbilitySystemComponent || !IsValid(AbilitySystemComponent))
	{
		return;
	}

	if (!HasAuthority()) return;

	URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent);
	if (!RPGASC) return;

	RPGASC->AddCharacterAbilities(StartupAbilities);
	RPGASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

void ARPGCharacterBase::Dissolve()
{
	if (IsValid(DissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMatInst);
		StartDissolveTimeline(DynamicMatInst);
	}
}


