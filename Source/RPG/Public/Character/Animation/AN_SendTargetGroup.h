// Copyright (c) 2025 RPG Yumi Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "AN_SendTargetGroup.generated.h"

/**
 * Animation Notify that sends target group data to the actor's Ability System Component
 * Creates sphere traces between socket locations to detect targets
 */
UCLASS()
class RPG_API UAN_SendTargetGroup : public UAnimNotify
{
	GENERATED_BODY()
	
public:	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	/** Gameplay cue tags to trigger on hit */
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	FGameplayTagContainer TriggerGameplayCueTags;

	/** Team attitude to target (Hostile, Friendly, Neutral) */
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TEnumAsByte<ETeamAttitude::Type> TargetTeam = ETeamAttitude::Hostile;

	/** Radius of the sphere sweep for target detection */
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	float SphereSweepRadius = 60.f;

	/** Whether to draw debug traces */
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	bool bDrawDebug = true;

	/** Whether to ignore the owner actor in traces */
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	bool bIgnoreOwner = true;

	/** Event tag to send with target data */
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	FGameplayTag EventTag;

	/** Socket names to trace between */
	UPROPERTY(EditAnywhere, Category = "Gameplay Ability")
	TArray<FName> TargetSocketNames;

	/** Send local gameplay cue to hit result */
	void SendLocalGameplayCue(const FHitResult& HitResult) const;
};

