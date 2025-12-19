#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractiveInterface.generated.h"

// Forward declarations
class ARPGCharacterBase;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractiveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface para objetos que podem ser interagidos
 */
class RPG_API IInteractiveInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/**
	 * Chamado quando um personagem interage com este objeto
	 * @param InteractingCharacter O personagem que está interagindo
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(ARPGCharacterBase* InteractingCharacter);

	/**
	 * Verifica se o personagem pode interagir com este objeto
	 * @param InteractingCharacter O personagem que quer interagir
	 * @return True se pode interagir, False caso contrário
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(ARPGCharacterBase* InteractingCharacter) const;

	/**
	 * Retorna o texto que deve ser exibido na UI de interação
	 * @param InteractingCharacter O personagem que está próximo
	 * @return O texto a ser exibido
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	FText GetInteractionText(ARPGCharacterBase* InteractingCharacter) const;
}; 