#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacter.h"
#include "AxixCharacter.generated.h"

/**
 * Classe específica para o personagem Axix.
 * Herda funcionalidades de ARPGCharacter (player character).
 * Lógica específica foi removida para usar sistema de templates/componentes.
 */
UCLASS()
class RPG_API AAxixCharacter : public ARPGCharacter
{
	GENERATED_BODY()

public:
	AAxixCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	
};
