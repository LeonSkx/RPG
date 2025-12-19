// Copyright Druid Mechanics

#include "AbilitySystem/Abilities/Combat/RPGProjectileSpell.h"
#include "Actor/RPGProjectile.h"
#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RPGGameplayTags.h"

void URPGProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URPGProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
    const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
    if (!bIsServer) return;

    // Verificar se o mundo ainda está válido (não está sendo destruído)
    UWorld* World = GetWorld();
    if (!IsValid(World) || World->bIsTearingDown)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RPGProjectileSpell] Tentativa de spawnar projétil durante teardown do mundo - abortando"));
        return;
    }

    const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
        GetAvatarActorFromActorInfo(),
        FRPGGameplayTags::Get().Montage_Attack_Weapon);
    FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

    FTransform SpawnTransform;
    SpawnTransform.SetLocation(SocketLocation);
    SpawnTransform.SetRotation(Rotation.Quaternion());

    ARPGProjectile* Projectile = GetWorld()->SpawnActorDeferred<ARPGProjectile>(
        ProjectileClass,
        SpawnTransform,
        GetOwningActorFromActorInfo(),
        Cast<APawn>(GetOwningActorFromActorInfo()),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
    if (SourceASC)
    {
        FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
        EffectContextHandle.SetAbility(this);
        EffectContextHandle.AddSourceObject(Projectile);
        TArray<TWeakObjectPtr<AActor>> Actors;
        Actors.Add(Projectile);
        EffectContextHandle.AddActors(Actors);
        FHitResult HitResult;
        HitResult.Location = ProjectileTargetLocation;
        EffectContextHandle.AddHitResult(HitResult);

        const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);

        const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageType, ScaledDamage);
        
        Projectile->DamageEffectSpecHandle = SpecHandle;
    }

    Projectile->FinishSpawning(SpawnTransform);
} 