#include "Character/RPGCharacter.h"

// Unreal Engine
#include "Engine/Engine.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"

// Game Framework
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Components
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

// Ability System
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "RPGGameplayTags.h"

// AI & Behavior
#include "AI/RPGPartyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

// Game Systems
#include "Game/RPGGameModeBase.h"
#include "Party/PartySubsystem.h"
#include "Progression/ProgressionSubsystem.h"

// Inventory & Equipment
#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquipmentComparisonComponent.h"
#include "Inventory/Equipment/EquippedItem.h"
#include "Inventory/Items/BaseItem.h"

// VFX
#include "NiagaraComponent.h"

// RPG
#include "RPG/RPG.h"

ARPGCharacter::ARPGCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Team = ERPGTeam::Player;
	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(false);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
	CameraBoom->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 60.f;
	FollowCamera->SetRelativeLocation(FVector(0.0f, 110.0f, 65.0f));



	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;
	
	PlayerClass = EPlayerClass::Elementalist;

	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>(TEXT("AttributeSet"));

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetIsReplicated(true);
		AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	}

	bIsInParty = false;
	bIsActivePartyMember = false;
	bCanJoinParty = true;

	// AbilityTargetBackstop removido - não é mais necessário
	MaxTargetingRange = 3500.f;

	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));
	EquipmentComparisonComponent = CreateDefaultSubobject<UEquipmentComparisonComponent>(TEXT("EquipmentComparisonComponent"));
	SkillEquipmentComponent = CreateDefaultSubobject<USkillEquipmentComponent>(TEXT("SkillEquipmentComponent"));
	
	bAutoPickupItems = true;
}

void ARPGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (!NewController)
	{
		return;
	}

	InitAbilityActorInfo();

	if (HasAuthority())
	{
		RPGPartyAIController = Cast<ARPGPartyAIController>(NewController);
		if (RPGPartyAIController && BehaviorTree)
		{
			RPGPartyAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
			RPGPartyAIController->RunBehaviorTree(BehaviorTree);
		}
	}
}

void ARPGCharacter::UnPossessed()
{
    Super::UnPossessed();

    if (HasAuthority())
    {
        // Verificar se o mundo ainda está válido antes de spawnar controller
        UWorld* World = GetWorld();
        if (IsValid(World) && !World->bIsTearingDown)
        {
            if (Controller == nullptr)
            {
                SpawnDefaultController();
            }
        }

        ARPGPartyAIController* PartyAI = Cast<ARPGPartyAIController>(Controller);
        if (PartyAI && BehaviorTree)
        {
            if (UBlackboardComponent* BB = PartyAI->GetBlackboardComponent())
            {
                if (!BB->GetBlackboardAsset())
                {
                    BB->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
                }
            }
            PartyAI->RunBehaviorTree(BehaviorTree);
        }
    }
}

void ARPGCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (EndPlayReason == EEndPlayReason::Destroyed)
	{
		// Sistema de save removido - não há mais persistência automática
	}

	if (AbilitySystemComponent)
	{
		// Limpar listener da tag Status_Aiming
		if (AimingTagDelegateHandle.IsValid())
		{
			AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Status_Aiming).Remove(AimingTagDelegateHandle);
			AimingTagDelegateHandle.Reset();
		}
		
		AbilitySystemComponent->CancelAllAbilities();
		AbilitySystemComponent->ClearAllAbilities();
		AbilitySystemComponent->DestroyComponent();
		AbilitySystemComponent = nullptr;
	}

	// Limpar timer de lerp da câmera
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CameraLerpTimerHandle);
	}

	// AbilityTargetBackstop removido - não é mais necessário

	Super::EndPlay(EndPlayReason);
}


void ARPGCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			ProgressionSubsystem->OnProgressionReady.AddDynamic(this, &ARPGCharacter::OnProgressionReady);
			ProgressionSubsystem->EnsureCharacterDataExists(this);
		}
	}
	
	// Backstop removido - não é mais necessário

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetGenerateOverlapEvents(true);
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ARPGCharacter::OnCapsuleOverlapBegin);
		GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ARPGCharacter::OnCapsuleOverlapEnd);
	}

	if (EquipmentComponent)
	{
		EquipmentComponent->OnItemEquipped.AddDynamic(this, &ARPGCharacter::HandleItemEquipped);
		EquipmentComponent->OnItemUnequipped.AddDynamic(this, &ARPGCharacter::HandleItemUnequipped);
	}
}



void ARPGCharacter::OnRep_Stunned()
{
}

void ARPGCharacter::OnRep_Burned()
{
}

void ARPGCharacter::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();
	
	if (!AbilitySystemComponent)
	{
		return;
	}
	
	// Registrar listener para a tag Status_Aiming para ativar lerp da câmera automaticamente
	if (!AimingTagDelegateHandle.IsValid())
	{
		AimingTagDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(
			FRPGGameplayTags::Get().Status_Aiming,
			EGameplayTagEventType::NewOrRemoved
		).AddUObject(this, &ARPGCharacter::OnAimingTagChanged);
	}
	
	if (HasAuthority() && AttributeSet && !bAttributesInitialized)
	{
		InitializeDefaultAttributes();
		AddCharacterAbilities();
		bAttributesInitialized = true;
		OnAttributesInitialized.Broadcast();
	}
	else if (HasAuthority() && AttributeSet)
	{
		OnAttributesInitialized.Broadcast();
	}
}


void ARPGCharacter::HandleItemEquipped(EEquipmentSlot Slot, UEquippedItem* EquippedItem)
{
	if (!EquippedItem)
	{
		return;
	}
	
	UItemDataAsset* Item = EquippedItem->GetItemData();
	if (!Item)
	{
		return;
	}

    if (!EquipmentComponent)
    {
        return;
    }
    FName SocketName = NAME_None;
    
    // Priorizar sockets de equipamento configurados no Item (se houver)
    if (Item && Item->EquipmentSockets.Num() > 0)
    {
        SocketName = Item->EquipmentSockets[0];
    }
    
    // Fallback para mapeamento do EquipmentComponent
    if (SocketName == NAME_None)
    {
        TArray<FName> SlotSockets = EquipmentComponent->GetAllSocketNamesForSlot(Slot);
        SocketName = SlotSockets.Num() > 0 ? SlotSockets[0] : NAME_None;
    }
    
    if (SocketName == NAME_None)
    {
        return;
    }

	SpawnEquipmentMesh(Slot, Item, SocketName);
}

void ARPGCharacter::HandleItemUnequipped(EEquipmentSlot Slot, const FInventoryItem& UnequippedItem)
{
	RemoveEquipmentMesh(Slot);
}

void ARPGCharacter::SpawnEquipmentMesh(EEquipmentSlot Slot, UItemDataAsset* Item, FName SocketName)
{
	if (!Item || SocketName == NAME_None)
	{
		return;
	}

	if (!Item->SkeletalMesh.IsNull())
	{
		USkeletalMesh* SkelMesh = Item->SkeletalMesh.LoadSynchronous();
		if (SkelMesh)
		{
			RemoveEquipmentMesh(Slot);

			FString ComponentName = FString::Printf(TEXT("EquippedMesh_%s"), *UEnum::GetValueAsString(Slot));
			USkeletalMeshComponent* NewMeshComponent = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), *ComponentName);
			if (NewMeshComponent)
			{
                NewMeshComponent->SetSkeletalMesh(SkelMesh);
                NewMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, SocketName);
                NewMeshComponent->SetRelativeTransform(FTransform::Identity);
				NewMeshComponent->RegisterComponent();
				NewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				EquippedMeshComponents.Add(Slot, NewMeshComponent);
                
                if (Item->bUseSecondarySocketVisual)
                {
                    // ✅ Usa segundo socket da lista EquipmentSockets do Item (não do EquipmentComponent)
                    if (Item->EquipmentSockets.Num() > 1)
                    {
                        const FName ExtraSocket = Item->EquipmentSockets[1];
                        if (ExtraSocket != NAME_None)
                        {
                            FString ExtraName = FString::Printf(TEXT("EquippedMesh_%s_Extra_%d"), *UEnum::GetValueAsString(Slot), 1);
                            USkeletalMeshComponent* ExtraComp = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), *ExtraName);
                            if (ExtraComp)
                            {
                                ExtraComp->SetSkeletalMesh(SkelMesh);
                                ExtraComp->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, ExtraSocket);
                                ExtraComp->SetRelativeTransform(FTransform::Identity);
                                ExtraComp->RegisterComponent();
                                ExtraComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                                FEquippedExtraMeshRef Ref; Ref.Mesh = ExtraComp; Ref.Slot = Slot;
                                EquippedExtraMeshes.Add(Ref);
                            }
                        }
                    }
                }
                return;
			}
			else
			{
				return;
			}
		}
	}

	// Sem SkeletalMesh configurada, não spawnar mesh
	return;
}

FName ARPGCharacter::GetCurrentWeaponCombatSocketName() const
{
    // Preferir primeiro socket de dano configurado no Item (se houver), depois mapeamento do EquipmentComponent
    if (EquipmentComponent)
    {
        if (UEquippedItem* EquippedWeapon = EquipmentComponent->GetEquippedItem(EEquipmentSlot::Weapon))
        {
            if (UItemDataAsset* ItemData = EquippedWeapon->GetItemData())
            {
                if (ItemData->DamageSockets.Num() > 0 && ItemData->DamageSockets[0] != NAME_None)
                {
                    return ItemData->DamageSockets[0];
                }
            }
        }

        const TArray<FName> Mapped = EquipmentComponent->GetAllSocketNamesForSlot(EEquipmentSlot::Weapon);
        if (Mapped.Num() > 0 && !Mapped[0].IsNone())
        {
            return Mapped[0];
        }
    }
    return NAME_None;
}

TArray<FName> ARPGCharacter::GetCurrentWeaponCombatSocketNames() const
{
    TArray<FName> Out;
    if (EquipmentComponent)
    {
        if (UEquippedItem* EquippedWeapon = EquipmentComponent->GetEquippedItem(EEquipmentSlot::Weapon))
        {
            if (UItemDataAsset* ItemData = EquippedWeapon->GetItemData())
            {
                // Primeiro: sockets de dano configurados no Item (prioridade máxima)
                for (const FName& SocketName : ItemData->DamageSockets)
                {
                    if (!SocketName.IsNone())
                    {
                        Out.Add(SocketName);
                    }
                }
            }
        }
        
        // Segundo: sockets mapeados pelo EquipmentComponent (fallback)
        const TArray<FName> Mapped = EquipmentComponent->GetAllSocketNamesForSlot(EEquipmentSlot::Weapon);
        for (const FName& Name : Mapped)
        {
            if (!Name.IsNone())
            {
                Out.AddUnique(Name);
            }
        }
    }
    return Out;
}

TArray<FVector> ARPGCharacter::GetCurrentWeaponCombatSocketLocations() const
{
    TArray<FVector> Out;
    if (EquipmentComponent)
    {
        if (UEquippedItem* EquippedWeapon = EquipmentComponent->GetEquippedItem(EEquipmentSlot::Weapon))
        {
            if (UItemDataAsset* ItemData = EquippedWeapon->GetItemData())
            {
                // PRIORIDADE 1: Procurar DamageSockets na PRIMEIRA ARMA
                if (EquippedMeshComponents.Contains(EEquipmentSlot::Weapon))
                {
                    if (const UMeshComponent* MeshComp = EquippedMeshComponents.FindRef(EEquipmentSlot::Weapon))
                    {
                        if (const USkeletalMeshComponent* SkelComp = Cast<USkeletalMeshComponent>(MeshComp))
                        {
                            for (const FName& SocketName : ItemData->DamageSockets)
                            {
                                if (!SocketName.IsNone() && SkelComp->DoesSocketExist(SocketName))
                                {
                                    Out.Add(SkelComp->GetSocketLocation(SocketName));
                                }
                            }
                        }
                    }
                }

                // PRIORIDADE 2: Procurar DamageSockets na SEGUNDA ARMA (se existir)
                if (EquippedExtraMeshes.Num() > 0)
                {
                    for (const FEquippedExtraMeshRef& ExtraMeshRef : EquippedExtraMeshes)
                    {
                        if (ExtraMeshRef.Slot == EEquipmentSlot::Weapon && ExtraMeshRef.Mesh)
                        {
                            if (USkeletalMeshComponent* ExtraSkelComp = Cast<USkeletalMeshComponent>(ExtraMeshRef.Mesh))
                            {
                                for (const FName& SocketName : ItemData->DamageSockets)
                                {
                                    if (!SocketName.IsNone() && ExtraSkelComp->DoesSocketExist(SocketName))
                                    {
                                        Out.Add(ExtraSkelComp->GetSocketLocation(SocketName));
                                    }
                                }
                            }
                        }
                    }
                }

                // PRIORIDADE 3: Fallback para personagem principal
                if (Out.Num() == 0)
                {
                    if (USkeletalMeshComponent* SkelMesh = GetMesh())
                    {
                        for (const FName& SocketName : ItemData->DamageSockets)
                        {
                            if (!SocketName.IsNone() && SkelMesh->DoesSocketExist(SocketName))
                            {
                                Out.Add(SkelMesh->GetSocketLocation(SocketName));
                            }
                        }
                    }
                }
            }
        }
    }
    return Out;
}

UMeshComponent* ARPGCharacter::GetEquippedMeshForSlot(EEquipmentSlot Slot) const
{
    if (EquippedMeshComponents.Contains(Slot))
    {
        return EquippedMeshComponents[Slot];
    }
    return nullptr;
}

void ARPGCharacter::RemoveEquipmentMesh(EEquipmentSlot Slot)
{
	if (UMeshComponent** FoundMesh = EquippedMeshComponents.Find(Slot))
	{
		UMeshComponent* MeshComponent = *FoundMesh;
		if (MeshComponent && ::IsValid(MeshComponent))
		{
			// Remover do personagem
			MeshComponent->DestroyComponent();
		}
		// Remover da referência
		EquippedMeshComponents.Remove(Slot);
	}

    // Remover extras associados ao slot
    if (EquippedExtraMeshes.Num() > 0)
    {
        for (int32 i = EquippedExtraMeshes.Num() - 1; i >= 0; --i)
        {
            if (EquippedExtraMeshes[i].Slot == Slot)
            {
                if (UMeshComponent* ExtraComp = EquippedExtraMeshes[i].Mesh)
                {
                    if (::IsValid(ExtraComp))
                    {
                        ExtraComp->DestroyComponent();
                    }
                }
                EquippedExtraMeshes.RemoveAt(i);
            }
        }
    }
}

void ARPGCharacter::OnCapsuleOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Verificar se a coleta automática está habilitada
	if (!bAutoPickupItems)
	{
		return;
	}

	// Verificar se é um BaseItem
	if (ABaseItem* Item = Cast<ABaseItem>(OtherActor))
	{
		if (Item->CanBePickedUp() && Item->GetItemData())
		{
			Item->OnPickedUp(this);
		}
	}
}

void ARPGCharacter::OnCapsuleOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABaseItem* Item = Cast<ABaseItem>(OtherActor))
	{
		// Overlap ended with BaseItem
	}
}

// UpdateTargetingBackstop removido - não é mais necessário

void ARPGCharacter::OnBecomeActivePartyMember()
{
    OnPartyStatusChanged.Broadcast(this);
}

void ARPGCharacter::OnBecomeInactivePartyMember()
{
    OnPartyStatusChanged.Broadcast(this);
}

FName ARPGCharacter::GetCharacterUniqueID() const
{
	// Usar o ID único fixo do character
	return CharacterUniqueID;
}



int32 ARPGCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			return ProgressionSubsystem->CalculateLevelFromXP(InXP);
		}
	}
	return 1;
}

int32 ARPGCharacter::GetXP_Implementation() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			return ProgressionSubsystem->GetCharacterXP(this);
		}
	}
	return 0;
}

int32 ARPGCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			return ProgressionSubsystem->GetAttributePointsReward(Level);
		}
	}
	return 1;
}

int32 ARPGCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			return ProgressionSubsystem->GetSpellPointsReward(Level);
		}
	}
	return 1;
}

void ARPGCharacter::AddToXP_Implementation(int32 InXP)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			ProgressionSubsystem->AddCharacterXP(this, InXP);
		}
	}
}

void ARPGCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			ProgressionSubsystem->SetCharacterLevel(this, InPlayerLevel);
		}
	}
}

void ARPGCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			ProgressionSubsystem->AddCharacterAttributePoints(this, InAttributePoints);
		}
	}
}

int32 ARPGCharacter::GetAttributePoints_Implementation() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			return ProgressionSubsystem->GetCharacterAttributePoints(this);
		}
	}
	return 0;
}

void ARPGCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			ProgressionSubsystem->AddCharacterSpellPoints(this, InSpellPoints);
		}
	}
}

int32 ARPGCharacter::GetSpellPoints_Implementation() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			return ProgressionSubsystem->GetCharacterSpellPoints(this);
		}
	}
	return 0;
}

void ARPGCharacter::LevelUp_Implementation()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UProgressionSubsystem>())
		{
			ProgressionSubsystem->LevelUpCharacter(this);
		}
	}
	
	MulticastLevelUpParticles();
}

void ARPGCharacter::MulticastLevelUpParticles_Implementation() const
{
	if (IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLocation = FollowCamera->GetComponentLocation();
		const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator ToCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation();
		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		LevelUpNiagaraComponent->Activate(true);
	}
}

void ARPGCharacter::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* ARPGCharacter::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}


bool ARPGCharacter::IsAlive() const
{
	return !IsDead_Implementation();
}

FString ARPGCharacter::GetCharacterName() const
{
    return CharacterName;
}

void ARPGCharacter::OnProgressionReady()
{
	if (!bAttributesInitialized)
	{
		InitAbilityActorInfo();
	}
}

void ARPGCharacter::SetPlayerClass(EPlayerClass NewClass)
{
	if (PlayerClass != NewClass)
	{
		PlayerClass = NewClass;
		OnPlayerClassChanged.Broadcast(NewClass);
		UE_LOG(LogTemp, Log, TEXT("PlayerClass alterada para: %d"), (int32)NewClass);
	}
}

// === CAMERA LERP SYSTEM ===

void ARPGCharacter::OnAimStateChanged(bool bIsAiming)
{
	// Aplicar/remover tag de aim (o lerp da câmera será ativado automaticamente pelo listener)
	if (AbilitySystemComponent)
	{
		const FGameplayTag AimTag = FRPGGameplayTags::Get().Status_Aiming;
		if (bIsAiming)
		{
			AbilitySystemComponent->AddLooseGameplayTag(AimTag);
		}
		else
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(AimTag);
		}
	}
}

void ARPGCharacter::OnAimingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	// Ativar/desativar lerp da câmera quando a tag Status_Aiming muda
	const bool bIsAiming = NewCount != 0;
	FVector Goal = bIsAiming ? CameraAimLocalOffset : CameraDefaultLocalOffset;
	LerpCameraToLocalOffsetLocation(Goal);
}

void ARPGCharacter::LerpCameraToLocalOffsetLocation(const FVector& Goal)
{
	if (!FollowCamera || !GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(CameraLerpTimerHandle);
	CameraLerpTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &ARPGCharacter::TickCameraLocalOffsetLerp, Goal)
	);
}

void ARPGCharacter::TickCameraLocalOffsetLerp(FVector Goal)
{
	if (!FollowCamera || !GetWorld())
	{
		return;
	}

	FVector CurrentLocalOffset = FollowCamera->GetRelativeLocation();
	
	// Se já está próximo o suficiente, definir posição final e parar
	if (FVector::Dist(CurrentLocalOffset, Goal) < 1.f)
	{
		FollowCamera->SetRelativeLocation(Goal);
		GetWorld()->GetTimerManager().ClearTimer(CameraLerpTimerHandle);
		return;
	}

	// Calcular lerp
	float LerpAlpha = FMath::Clamp(GetWorld()->GetDeltaSeconds() * CameraLerpSpeed, 0.f, 1.f);
	FVector NewLocalOffset = FMath::Lerp(CurrentLocalOffset, Goal, LerpAlpha);
	FollowCamera->SetRelativeLocation(NewLocalOffset);
	
	// Agendar próximo frame
	GetWorld()->GetTimerManager().ClearTimer(CameraLerpTimerHandle);
	CameraLerpTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &ARPGCharacter::TickCameraLocalOffsetLerp, Goal)
	);
}


