#include "Character/Yumi/YumiCharacter.h"
// Base already includes CharacterClassInfo via RPGCharacterBase -> OK

AYumiCharacter::AYumiCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    // Definir ID único para Yumi
    CharacterUniqueID = FName("Yumi_ID");
    // Definir nome exibido
    CharacterName = TEXT("Yumi");
    // Definir classe do personagem
    PlayerClass = EPlayerClass::Elementalist;
}

void AYumiCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Initialization logic for Yumi
}

void AYumiCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Per-frame logic for Yumi
} 