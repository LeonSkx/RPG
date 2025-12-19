// Copyright Druid Mechanics

#include "Game/RPGGameInstance.h"
#include "Quest/QuestSubsystem.h"

void URPGGameInstance::Init()
{
	Super::Init();

	// Configurar Quest Subsystem automaticamente
	if (UQuestSubsystem* QuestSubsystem = GetSubsystem<UQuestSubsystem>())
	{
		QuestSubsystem->SetQuestDataAssets(QuestDataAssets);
	}
}

// Sistema de save removido do projeto

