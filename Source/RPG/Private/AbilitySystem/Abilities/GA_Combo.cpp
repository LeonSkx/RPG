// Copyright (c) 2025 RPG Yumi Project. All rights reserved.

#include "AbilitySystem/Abilities/GA_Combo.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "RPGGameplayTags.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Interaction/CombatInterface.h"
#include "Character/RPGCharacter.h"
#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquippedItem.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Actor/RPGProjectile.h"


UGA_Combo::UGA_Combo()
{
	// Usar a tag de ataque do projeto
	SetAssetTags(FGameplayTagContainer(FRPGGameplayTags::Get().Abilities_Attack));
	BlockAbilitiesWithTag.AddTag(FRPGGameplayTags::Get().Abilities_Attack);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// Configurar o input tag (ex: InputTag_LMB para ataque básico)
	// O InputPressed será chamado automaticamente quando este input for pressionado
	StartupInputTag = FRPGGameplayTags::Get().InputTag_LMB;
}

void UGA_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
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

		// WaitGameplayEvent com OnlyMatchExact=false para aceitar tags filhas
		// Exemplo: "Ability.Combo.Change" aceita "Ability.Combo.Change.Combo02"
		UAbilityTask_WaitGameplayEvent* WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, 
			GetComboChangedEventTag(), 
			nullptr,  // OptionalExternalOwner (nullptr = usa o owner da ability)
			false,    // OnlyMatchExact = false (aceita tags filhas)
			false     // OnlyTriggerOnce = false (pode receber múltiplos eventos)
		);
		WaitComboChangeEventTask->EventReceived.AddDynamic(this, &UGA_Combo::ComboChangedEventReceived);
		WaitComboChangeEventTask->ReadyForActivation();
		
		UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Waiting for combo change events with tag: %s"), *GetComboChangedEventTag().ToString());
		
		// Notifica BP que o primeiro passo do combo iniciou
		if (IsPlayerController())
		{
			OnComboStepStarted_Player();
		}
		else if (IsPartyAIController())
		{
			OnComboStepStarted_AI();
		}
	}

	NextComboName = NAME_None;
	bComboWindowOpen = false;
}

FGameplayTag UGA_Combo::GetComboChangedEventTag()
{
	// Usar UGameplayTagsManager para funcionar com tags do Editor e nativas
	FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Ability.Combo.Change"), false);
	if (!Tag.IsValid())
	{
		// Fallback: tentar usar a tag nativa do sistema
		Tag = FRPGGameplayTags::Get().Ability_Combo_Change;
	}
	return Tag;
}

FGameplayTag UGA_Combo::GetComboChangedEventEndTag()
{
	// Usar UGameplayTagsManager para funcionar com tags do Editor e nativas
	FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(FName("Ability.Combo.Change.End"), false);
	if (!Tag.IsValid())
	{
		// Fallback: tentar usar a tag nativa do sistema
		Tag = FRPGGameplayTags::Get().Ability_Combo_Change_End;
	}
	return Tag;
}

void UGA_Combo::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	
	// Se não há próximo combo disponível, ignorar o input (não fazer nada)
	// Isso evita reativações indesejadas e warnings desnecessários
	if (NextComboName == NAME_None)
	{
		// Input foi pressionado mas não há combo disponível - comportamento normal
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[GA_Combo] InputPressed - NextComboName: %s, bComboWindowOpen: %s"), 
		*NextComboName.ToString(),
		bComboWindowOpen ? TEXT("true") : TEXT("false"));
	
	// Tentar fazer commit do combo se houver um próximo combo definido
	TryCommitCombo();
}

void UGA_Combo::TryCommitCombo()
{
	if (NextComboName == NAME_None)
	{
		// Isso não deveria acontecer (já foi verificado no InputPressed), mas por segurança
		return;
	}

	// Obter AnimInstance do Character
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!Character || !Character->GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_Combo] TryCommitCombo - Invalid Character or Mesh"));
		return;
	}

	UAnimInstance* OwnerAnimInst = Character->GetMesh()->GetAnimInstance();
	if (!OwnerAnimInst || !ComboMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_Combo] TryCommitCombo - Invalid AnimInstance or ComboMontage"));
		return;
	}

	FName CurrentSection = OwnerAnimInst->Montage_GetCurrentSection(ComboMontage);
	UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Changing section from '%s' to '%s'"), 
		CurrentSection.IsNone() ? TEXT("None") : *CurrentSection.ToString(), 
		*NextComboName.ToString());

	OwnerAnimInst->Montage_SetNextSection(CurrentSection, NextComboName, ComboMontage);
	
	// Resetar após commit bem-sucedido
	FName CommittedCombo = NextComboName;
	NextComboName = NAME_None;
	bComboWindowOpen = false;
	
	UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Combo committed successfully: %s"), *CommittedCombo.ToString());
}

void UGA_Combo::ComboChangedEventReceived(FGameplayEventData Data)
{
	FGameplayTag EventTag = Data.EventTag;

	UE_LOG(LogTemp, Log, TEXT("[GA_Combo] ComboChangedEventReceived - EventTag: %s"), *EventTag.ToString());

	FGameplayTag EndTag = GetComboChangedEventEndTag();
	if (EventTag == EndTag || EventTag.MatchesTag(EndTag))
	{
		UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Combo window closed"));
		bComboWindowOpen = false;
		// NÃO resetar NextComboName aqui - pode ter sido pressionado antes do End
		// O NextComboName será resetado depois do commit ou no próximo combo
		return;
	}
	
	// Se recebeu uma tag de mudança de combo, a janela está aberta
	bComboWindowOpen = true;
	
	// Extrair o nome da seção da tag
	// Exemplo: "Ability.Combo.Change.Combo02" -> "Combo02"
	// Ou: "Ability.Combo.Combo02" -> "Combo02" (se a tag foi criada diretamente no editor)
	FString TagString = EventTag.ToString();
	FString BaseTagString = GetComboChangedEventTag().ToString();
	
	// Verificar se a tag começa com a base tag
	if (TagString.StartsWith(BaseTagString))
	{
		// Remover a tag base e o ponto, deixando apenas o nome da seção
		FString SectionName = TagString.RightChop(BaseTagString.Len() + 1); // +1 para remover o ponto
		NextComboName = FName(*SectionName);
		UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Next combo section set to: %s (from tag: %s)"), 
			*NextComboName.ToString(), *TagString);
	}
	else
	{
		// Tentar método alternativo: extrair o último segmento da tag
		// Isso funciona para tags criadas no editor como "Ability.Combo.Combo02"
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
		
		if (TagNames.Num() > 0)
		{
			// Pegar o último segmento (nome da seção)
			NextComboName = TagNames.Last();
			UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Next combo section (extracted from segments): %s (from tag: %s)"), 
				*NextComboName.ToString(), *TagString);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GA_Combo] Failed to extract section name from tag: %s"), *EventTag.ToString());
		}
	}
}

FName UGA_Combo::GetCurrentWeaponSocketName() const
{
	if (const ARPGCharacter* PC = Cast<ARPGCharacter>(GetAvatarActorFromActorInfo()))
	{
		return PC->GetCurrentWeaponCombatSocketName();
	}
	return NAME_None;
}

FVector UGA_Combo::GetCurrentWeaponSocketLocation() const
{
	const ARPGCharacter* PC = Cast<ARPGCharacter>(GetAvatarActorFromActorInfo());
	if (!PC || !PC->GetMesh()) return FVector::ZeroVector;

	const FName SocketName = PC->GetCurrentWeaponCombatSocketName();
	if (SocketName == NAME_None) return FVector::ZeroVector;

	// 1) Tentar skeletical weapon via interface
	if (USkeletalMeshComponent* WeaponSkel = ICombatInterface::Execute_GetWeapon(const_cast<ARPGCharacter*>(PC)))
	{
		if (WeaponSkel->DoesSocketExist(SocketName))
		{
			return WeaponSkel->GetSocketLocation(SocketName);
		}
	}

	// 2) Tentar componentes reais do equipamento e múltiplos sockets
	if (const UMeshComponent* EquipMesh = PC->GetEquippedMeshForSlot(EEquipmentSlot::Weapon))
	{
		// Primeiro, o socket principal
		if (EquipMesh->DoesSocketExist(SocketName))
		{
			return EquipMesh->GetSocketLocation(SocketName);
		}
		// Depois, quaisquer adicionais configurados
		// (novo sistema removido) sem iterar alternativos
	}

	// 3) Tentar no mesh do personagem como fallback
	if (PC->GetMesh()->DoesSocketExist(SocketName))
	{
		return PC->GetMesh()->GetSocketLocation(SocketName);
	}

	return FVector::ZeroVector;
}

TArray<FName> UGA_Combo::GetConfiguredWeaponSocketNames() const
{
	TArray<FName> Out;
	const ARPGCharacter* PC = Cast<ARPGCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return Out;
	if (const UEquipmentComponent* Equip = PC->GetEquipmentComponent())
	{
		Out = Equip->GetAllSocketNamesForSlot(EEquipmentSlot::Weapon);
	}
	return Out;
}

TArray<FName> UGA_Combo::GetExistingWeaponSocketNames() const
{
	TArray<FName> Names;
	const ARPGCharacter* PC = Cast<ARPGCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return Names;
	
	// PRIORIDADE 1: Adicionar DamageSockets do item equipado
	if (const UEquipmentComponent* Equip = PC->GetEquipmentComponent())
	{
		if (UEquippedItem* EquippedWeapon = Equip->GetEquippedItem(EEquipmentSlot::Weapon))
		{
			if (UItemDataAsset* ItemData = EquippedWeapon->GetItemData())
			{
				for (const FName& SocketName : ItemData->DamageSockets)
				{
					if (!SocketName.IsNone())
					{
						Names.AddUnique(SocketName);
					}
				}
			}
		}
	}
	
	// PRIORIDADE 2: Adicionar sockets configurados no EquipmentComponent
	TArray<FName> ConfiguredNames = GetConfiguredWeaponSocketNames();
	for (const FName& Name : ConfiguredNames)
	{
		if (!Name.IsNone())
		{
			Names.AddUnique(Name);
		}
	}
	
	// Verificar quais sockets realmente existem nos meshes
	TArray<FName> Existing;
	const UMeshComponent* EquipMesh = PC->GetEquippedMeshForSlot(EEquipmentSlot::Weapon);
	USkeletalMeshComponent* WeaponSkel = ICombatInterface::Execute_GetWeapon(const_cast<ARPGCharacter*>(PC));
	USkeletalMeshComponent* CharMesh = PC->GetMesh();

	for (const FName& Name : Names)
	{
		if (Name.IsNone()) continue;
		if (WeaponSkel && WeaponSkel->DoesSocketExist(Name)) { Existing.Add(Name); continue; }
		if (EquipMesh && EquipMesh->DoesSocketExist(Name))    { Existing.Add(Name); continue; }
		if (CharMesh && CharMesh->DoesSocketExist(Name))      { Existing.Add(Name); continue; }
	}
	return Existing;
}

FVector UGA_Combo::GetWeaponSocketLocationByName(FName SocketName) const
{
	if (SocketName.IsNone()) return FVector::ZeroVector;
	const ARPGCharacter* PC = Cast<ARPGCharacter>(GetAvatarActorFromActorInfo());
	if (!PC) return FVector::ZeroVector;

	if (USkeletalMeshComponent* WeaponSkel = ICombatInterface::Execute_GetWeapon(const_cast<ARPGCharacter*>(PC)))
	{
		if (WeaponSkel->DoesSocketExist(SocketName))
		{
			return WeaponSkel->GetSocketLocation(SocketName);
		}
	}
	if (const UMeshComponent* EquipMesh = PC->GetEquippedMeshForSlot(EEquipmentSlot::Weapon))
	{
		if (EquipMesh->DoesSocketExist(SocketName))
		{
			return EquipMesh->GetSocketLocation(SocketName);
		}
	}
	if (USkeletalMeshComponent* CharMesh = PC->GetMesh())
	{
		if (CharMesh->DoesSocketExist(SocketName))
		{
			return CharMesh->GetSocketLocation(SocketName);
		}
	}
	return FVector::ZeroVector;
}

// Cópia adaptada de URPGProjectileSpell::SpawnProjectile, sem spawn: apenas prepara o contexto de dano relativo ao alvo
void UGA_Combo::DamageLocation(const FVector& ProjectileTargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
		GetAvatarActorFromActorInfo(),
		FRPGGameplayTags::Get().Montage_Attack_Weapon);
	UE_LOG(LogTemp, Log, TEXT("[GA_Combo] DamageLocation -> Pos: (%.1f, %.1f, %.1f)"),
		SocketLocation.X, SocketLocation.Y, SocketLocation.Z);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());

	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	if (SourceASC)
	{
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.SetAbility(this);
		// Nota: Sem projectile; se desejar, podemos adicionar atores alvos aqui
		FHitResult HitResult;
		HitResult.Location = ProjectileTargetLocation;
		EffectContextHandle.AddHitResult(HitResult);

		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
		const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageType, ScaledDamage);

		// Aqui você pode aplicar o dano a um alvo específico, se tiver o ASC alvo.
		// Ex.: URPGAbilitySystemLibrary::ApplyDamage(WorldCtx, SpecHandle, SourceTags, SourceActor, TargetActor);
	}
}

// === SISTEMA UNIFICADO: SPAWN DE PROJÉTIL ===
void UGA_Combo::SpawnProjectile(const FVector& PlayerLocation, const FVector& ActorLocation, 
                                bool bUseManualSpawnOrigin, const FVector& ManualSpawnOrigin)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	// Verificar se o mundo ainda está válido (não está sendo destruído)
	UWorld* World = GetWorld();
	if (!IsValid(World) || World->bIsTearingDown)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_Combo] Tentativa de spawnar projétil durante teardown do mundo - abortando"));
		return;
	}

	// Determinar a origem do spawn PRIMEIRO: manual ou socket da arma
	FVector SpawnOrigin = FVector::ZeroVector;
	if (bUseManualSpawnOrigin)
	{
		SpawnOrigin = ManualSpawnOrigin;
		UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Usando ManualSpawnOrigin: (%.1f, %.1f, %.1f)"),
			SpawnOrigin.X, SpawnOrigin.Y, SpawnOrigin.Z);
	}
	else
	{
		SpawnOrigin = ICombatInterface::Execute_GetCombatSocketLocation(
			GetAvatarActorFromActorInfo(),
			FRPGGameplayTags::Get().Montage_Attack_Weapon);
		UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Usando SocketLocation: (%.1f, %.1f, %.1f)"),
			SpawnOrigin.X, SpawnOrigin.Y, SpawnOrigin.Z);
	}

	// Escolher automaticamente qual localização usar baseado no contexto
	FVector ProjectileTargetLocation = FVector::ZeroVector;
	
	if (IsPlayerController())
	{
		// PLAYER: Usar PlayerLocation (mouse/crosshair)
		ProjectileTargetLocation = PlayerLocation;
		UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Player - Usando PlayerLocation: (%.1f, %.1f, %.1f)"),
			ProjectileTargetLocation.X, ProjectileTargetLocation.Y, ProjectileTargetLocation.Z);
	}
	else if (IsPartyAIController())
	{
		// IA: Usar ActorLocation (alvo de combate)
		ProjectileTargetLocation = ActorLocation;
		UE_LOG(LogTemp, Log, TEXT("[GA_Combo] AI - Usando ActorLocation: (%.1f, %.1f, %.1f)"),
			ProjectileTargetLocation.X, ProjectileTargetLocation.Y, ProjectileTargetLocation.Z);
	}
	else
	{
		// Fallback: tentar usar PlayerLocation primeiro, depois ActorLocation
		if (PlayerLocation != FVector::ZeroVector)
		{
			ProjectileTargetLocation = PlayerLocation;
			UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Fallback - Usando PlayerLocation: (%.1f, %.1f, %.1f)"),
				ProjectileTargetLocation.X, ProjectileTargetLocation.Y, ProjectileTargetLocation.Z);
		}
		else if (ActorLocation != FVector::ZeroVector)
		{
			ProjectileTargetLocation = ActorLocation;
			UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Fallback - Usando ActorLocation: (%.1f, %.1f, %.1f)"),
				ProjectileTargetLocation.X, ProjectileTargetLocation.Y, ProjectileTargetLocation.Z);
		}
		else
		{
			// Último fallback: direção frontal do personagem
			if (AActor* Avatar = GetAvatarActorFromActorInfo())
			{
				ProjectileTargetLocation = Avatar->GetActorLocation() + (Avatar->GetActorForwardVector() * 1000.0f);
				UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Último Fallback - Direção Frontal: (%.1f, %.1f, %.1f)"),
					ProjectileTargetLocation.X, ProjectileTargetLocation.Y, ProjectileTargetLocation.Z);
			}
		}
	}

	// Se não usar projéteis, aplicar dano direto
	if (!bUseProjectiles)
	{
		DamageLocation(ProjectileTargetLocation);
		return;
	}

	// Se usar projéteis mas não tiver classe configurada, aplicar dano direto
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GA_Combo] bUseProjectiles=true mas ProjectileClass não configurada! Aplicando dano direto."));
		DamageLocation(ProjectileTargetLocation);
		return;
	}

	FRotator Rotation = (ProjectileTargetLocation - SpawnOrigin).Rotation();

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnOrigin);
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
	UE_LOG(LogTemp, Log, TEXT("[GA_Combo] Projétil spawnado na posição: (%.1f, %.1f, %.1f)"),
		SpawnOrigin.X, SpawnOrigin.Y, SpawnOrigin.Z);
}