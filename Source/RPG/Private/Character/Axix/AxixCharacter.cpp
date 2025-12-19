#include "Character/Axix/AxixCharacter.h"

AAxixCharacter::AAxixCharacter()
{
	// Definir um ID único para o Axix
	CharacterUniqueID = FName("Axix_ID");
    CharacterName = TEXT("Axix");
    // Definir classe do personagem
    PlayerClass = EPlayerClass::Automaton;
}

void AAxixCharacter::BeginPlay()
{
	Super::BeginPlay();
}
