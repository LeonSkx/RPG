#include "Character/Yoshino/YoshinoCharacter.h"

AYoshinoCharacter::AYoshinoCharacter()
{
	// Definir um ID único para a Yoshino
	CharacterUniqueID = FName("Yoshino_ID");
    CharacterName = TEXT("Yoshino");
    // Definir classe do personagem
    PlayerClass = EPlayerClass::Scout;
}

void AYoshinoCharacter::BeginPlay()
{
	Super::BeginPlay();
}
