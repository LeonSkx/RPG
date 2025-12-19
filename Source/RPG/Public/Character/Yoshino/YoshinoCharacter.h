#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacter.h"
#include "YoshinoCharacter.generated.h"

/**
 * Classe específica para o personagem Yoshino.
 * Herda funcionalidades de ARPGCharacter (player character).
 * Lógica específica foi removida para usar sistema de templates/componentes.
 */
UCLASS()
class RPG_API AYoshinoCharacter : public ARPGCharacter
{
	GENERATED_BODY()

public:
	AYoshinoCharacter();

protected:
	virtual void BeginPlay() override;

	// Remover overrides e propriedades específicas
	// virtual void Tick(float DeltaTime) override;
	// virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// virtual void PerformBasicAttack() override;
	// ... outros ...
}; 
