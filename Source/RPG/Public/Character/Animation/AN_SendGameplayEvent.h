// Copyright (c) 2025 RPG Yumi Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_SendGameplayEvent.generated.h"

/**
 * Animation Notify that sends a gameplay event to the actor's Ability System Component
 */
UCLASS()
class RPG_API UAN_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SendGameplayEvent();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	/** The gameplay tag to send as an event */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Event")
	FGameplayTag EventTag;
};

