#include "Character/Shimako/ShimakoCharacter.h"

AShimakoCharacter::AShimakoCharacter()
{
    // Definir um ID único para a Shimako
    CharacterUniqueID = FName("Shimako_ID");
    // Definir nome exibido
    CharacterName = TEXT("Shimako");
    // Definir classe do personagem
    PlayerClass = EPlayerClass::Guardian;
}

void AShimakoCharacter::BeginPlay()
{
    Super::BeginPlay(); // Chama ARPGCharacter::BeginPlay que carrega o template
}