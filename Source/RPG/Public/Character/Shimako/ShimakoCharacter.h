#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacter.h"
#include "ShimakoCharacter.generated.h"

/**
 * Classe específica para o personagem Shimako.
 * Herda funcionalidades de ARPGCharacter (player character).
 * Lógica específica foi removida para usar sistema de templates/componentes.
 */
UCLASS()
class RPG_API AShimakoCharacter : public ARPGCharacter
{
	GENERATED_BODY()

public:
	AShimakoCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	// Remover overrides e propriedades específicas
	// virtual void Tick(float DeltaTime) override;
	// virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// virtual void PerformBasicAttack() override;
	// ... outros ...
};
