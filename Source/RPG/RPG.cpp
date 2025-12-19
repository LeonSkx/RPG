// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPG.h"
#include "Modules/ModuleManager.h"
#include "AbilitySystemGlobals.h"

void FRPGModule::StartupModule()
{
	// Inicializar AbilitySystemGlobals para evitar problemas de inicialização
	UAbilitySystemGlobals::Get().InitGlobalData();
}

void FRPGModule::ShutdownModule()
{
	// Cleanup se necessário
}

IMPLEMENT_PRIMARY_GAME_MODULE(FRPGModule, RPG, "RPG");
