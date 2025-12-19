// Copyright Druid Mechanics


#include "Game/RPGGameModeBase.h"
#include "Engine/DataTable.h"
#include "Progression/SkillTreeSubsystem.h"
#include "Game/RPGGameInstance.h"

ARPGGameModeBase::ARPGGameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARPGGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	// Configurar Skill Trees por personagem
	SetupSkillTrees();
}

void ARPGGameModeBase::SetupSkillTrees()
{
	if (URPGGameInstance* RPGGI = Cast<URPGGameInstance>(GetGameInstance()))
	{
		if (USkillTreeSubsystem* SkillTreeSubsystem = RPGGI->GetSubsystem<USkillTreeSubsystem>())
		{
			// Configurar Data Tables específicos por personagem
			for (const auto& CharacterTable : CharacterSkillTables)
			{
				const FName& CharacterID = CharacterTable.Key;
				UDataTable* DataTable = CharacterTable.Value;
				
				if (DataTable)
				{
					SkillTreeSubsystem->SetCharacterSkillTable(CharacterID, DataTable);
				}
			}
		}
	}
}

