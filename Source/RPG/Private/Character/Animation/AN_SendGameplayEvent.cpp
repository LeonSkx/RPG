// Copyright (c) 2025 RPG Yumi Project. All rights reserved.

#include "Character/Animation/AN_SendGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"

UAN_SendGameplayEvent::UAN_SendGameplayEvent()
{
	EventTag = FGameplayTag::EmptyTag;
}

void UAN_SendGameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp->GetOwner())
		return;

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (!OwnerASC)
		return;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag, FGameplayEventData());
}

FString UAN_SendGameplayEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
		
		// Verificar se o array não está vazio antes de acessar o último elemento
		if (TagNames.Num() > 0)
		{
			return TagNames.Last().ToString();
		}
		
		// Se o array estiver vazio, retornar a string da tag completa como fallback
		return EventTag.ToString();
	}

	return "None";
}

