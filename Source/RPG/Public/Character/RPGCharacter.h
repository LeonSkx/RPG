#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "Interaction/PartyInterface.h"
#include "Inventory/Equipment/EquipmentComponent.h"
#include "Inventory/Equipment/EquipmentComparisonComponent.h"
#include "Components/SkillEquipmentComponent.h"
#include "Character/PlayerClassInfo.h"
#include "RPGCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartyStatusChanged, ARPGCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttributesInitialized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerClassChanged, EPlayerClass, NewClass);

class UNiagaraComponent;
class UNiagaraSystem;
class UCameraComponent;
class USpringArmComponent;
class UPartySubsystem;
class UItemDataAsset;
class UEquippedItem;
class UMeshComponent;
class UInventoryComponent;
class UEquipmentComponent;
class ABaseItem;
class UBehaviorTree;
class ARPGPartyAIController;

USTRUCT(BlueprintType)
struct FEquippedExtraMeshRef
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UMeshComponent> Mesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EEquipmentSlot Slot = EEquipmentSlot::Weapon;
};

UCLASS()
class RPG_API ARPGCharacter 
    : public ARPGCharacterBase
    , public IPlayerInterface
    , public IPartyInterface
{
    GENERATED_BODY()

public:
    ARPGCharacter(const FObjectInitializer& ObjectInitializer);

    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;

    /** Obtém a classe do personagem */
    UFUNCTION(BlueprintPure, Category = "Character|Class")
    EPlayerClass GetPlayerClass() const { return PlayerClass; }

    /** Define a classe do personagem e dispara delegate */
    UFUNCTION(BlueprintCallable, Category = "Character|Class")
    void SetPlayerClass(EPlayerClass NewClass);

    UFUNCTION(BlueprintPure, Category = "Combat|Sockets")
    FName GetCurrentWeaponCombatSocketName() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Sockets")
    TArray<FName> GetCurrentWeaponCombatSocketNames() const;

    UFUNCTION(BlueprintPure, Category = "Combat|Sockets")
    TArray<FVector> GetCurrentWeaponCombatSocketLocations() const;

    UFUNCTION(BlueprintPure, Category = "Equipment|Runtime")
    UMeshComponent* GetEquippedMeshForSlot(EEquipmentSlot Slot) const;

    UFUNCTION(BlueprintPure, Category = "Party")
    FName GetCharacterUniqueID() const;
    
    UFUNCTION(BlueprintPure, Category = "Party")
    FString GetCharacterName() const;

    UFUNCTION(BlueprintPure, Category = "Equipment")
    UEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

    UFUNCTION(BlueprintPure, Category = "Equipment")
    UEquipmentComparisonComponent* GetEquipmentComparisonComponent() const { return EquipmentComparisonComponent; }

    UFUNCTION(BlueprintPure, Category = "Skills")
    USkillEquipmentComponent* GetSkillEquipmentComponent() const { return SkillEquipmentComponent; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlive() const;

    // GetAbilityTargetBackstop removido - não é mais necessário

    UFUNCTION(BlueprintCallable, Category = "Party")
    void OnBecomeActivePartyMember();

    UFUNCTION(BlueprintCallable, Category = "Party")
    void OnBecomeInactivePartyMember();

    // UpdateTargetingBackstop removido - não é mais necessário

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void SpawnEquipmentMesh(EEquipmentSlot Slot, UItemDataAsset* Item, FName SocketName);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void RemoveEquipmentMesh(EEquipmentSlot Slot);

    UFUNCTION()
    void OnProgressionReady();

    UFUNCTION()
    void OnCapsuleOverlapBegin(
        UPrimitiveComponent* OverlappedComp, 
        AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, 
        int32 OtherBodyIndex, 
        bool bFromSweep, 
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnCapsuleOverlapEnd(
        UPrimitiveComponent* OverlappedComp, 
        AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, 
        int32 OtherBodyIndex
    );

    UFUNCTION(NetMulticast, Reliable)
    void MulticastLevelUpParticles() const;

    // Interfaces ---------------------------------------------------

    // Player Interface
    virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
    virtual int32 GetXP_Implementation() const override;
    virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
    virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;
    virtual void AddToXP_Implementation(int32 InXP) override;
    virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
    virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;
    virtual int32 GetAttributePoints_Implementation() const override;
    virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;
    virtual int32 GetSpellPoints_Implementation() const override;
    virtual void LevelUp_Implementation() override;


    // Party Interface
    virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
    virtual AActor* GetCombatTarget_Implementation() const override;

    // Rep Notifies
    virtual void OnRep_Stunned() override;
    virtual void OnRep_Burned() override;
    

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Character|Events")
    FOnPartyStatusChanged OnPartyStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Character|Events")
    FOnAttributesInitialized OnAttributesInitialized;

    UPROPERTY(BlueprintAssignable, Category = "Character|Events")
    FOnPlayerClassChanged OnPlayerClassChanged;

    // Data
    UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

    // Variáveis públicas -------------------------------------------
    UPROPERTY(EditDefaultsOnly)
    float DeathTime = 5.f;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    bool bIsInParty = false;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    bool bIsActivePartyMember = false;

    UPROPERTY(BlueprintReadOnly, Category = "Party")
    bool bCanJoinParty = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|Auto Pickup")
    bool bAutoPickupItems = true;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TObjectPtr<AActor> CombatTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    TObjectPtr<UNiagaraSystem> LevelUpEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCameraComponent> FollowCamera;

    

    // AbilityTargetBackstop removido - não é mais necessário

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Targeting")
    float MaxTargetingRange = 3500.f;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UEquipmentComponent* EquipmentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UEquipmentComparisonComponent* EquipmentComparisonComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Skills")
    TObjectPtr<USkillEquipmentComponent> SkillEquipmentComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Runtime")
    TMap<EEquipmentSlot, UMeshComponent*> EquippedMeshComponents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Runtime")
    TArray<FEquippedExtraMeshRef> EquippedExtraMeshes;

    UFUNCTION()
    void HandleItemEquipped(EEquipmentSlot Slot, UEquippedItem* EquippedItem);

    UFUNCTION()
    void HandleItemUnequipped(EEquipmentSlot Slot, const FInventoryItem& UnequippedItem);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    FString CharacterName = TEXT("Player");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
    FName CharacterUniqueID = TEXT("Player");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Class")
    EPlayerClass PlayerClass = EPlayerClass::Elementalist;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY()
    TObjectPtr<ARPGPartyAIController> RPGPartyAIController;

private:
    virtual void InitAbilityActorInfo() override;

    FTimerHandle DeathTimer;
    bool bAttributesInitialized = false;
};
