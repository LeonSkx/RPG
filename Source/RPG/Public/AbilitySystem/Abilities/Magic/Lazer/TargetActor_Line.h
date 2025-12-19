#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"

#include "GenericTeamAgentInterface.h"
#include "TargetActor_Line.generated.h"

UCLASS()
class RPG_API ATargetActor_Line : public AGameplayAbilityTargetActor, public IGenericTeamAgentInterface
{
    GENERATED_BODY()

public:
    ATargetActor_Line();

    void ConfigureTargetSetting(
        float NewTargetRange,
        float NewDetectionCylinderRadius,
        float NewTargetingInterval,
        FGenericTeamId OwnerTeamId,
        bool bShouldDrawDebug
    );

    // IGenericTeamAgentInterface
    virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
    virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }

    // AGameplayAbilityTargetActor
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void StartTargeting(UGameplayAbility* Ability) override;
    virtual void Tick(float DeltaTime) override;
    virtual void BeginDestroy() override;

protected:
    UPROPERTY(Replicated)
    float TargetRange;

    UPROPERTY(Replicated)
    float DetectionCylinderRadius;

    UPROPERTY()
    float TargetingInterval;

    UPROPERTY(Replicated)
    FGenericTeamId TeamId;

    UPROPERTY()
    bool bDrawDebug;

    UPROPERTY(Replicated)
    const AActor* AvatarActor;

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    FName LazerFXLengthParamName = "Length";

    UPROPERTY(VisibleDefaultsOnly, Category = "Component")
    class USceneComponent* RootComp;

    UPROPERTY(VisibleDefaultsOnly, Category = "Component")
    class UNiagaraComponent* LazerVFX;

    UPROPERTY(VisibleDefaultsOnly, Category = "Component")
    class USphereComponent* TargetEndDetectionSphere;

private:
    FTimerHandle PeriodicalTargetingTimerHandle;

    void DoTargetCheckAndReport();

    void UpdateTargetTrace();

    bool ShouldReportActorAsTarget(const AActor* ActorToCheck) const;
};

