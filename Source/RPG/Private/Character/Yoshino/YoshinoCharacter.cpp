#include "Character/Yoshino/YoshinoCharacter.h"

AYoshinoCharacter::AYoshinoCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
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
