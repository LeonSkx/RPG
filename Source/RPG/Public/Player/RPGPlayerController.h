// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "RPGPlayerController.generated.h"

struct FInputActionValue;
class UInputMappingContext;
class UInputAction;
class URPGInputConfig;
class URPGAbilitySystemComponent;

class UGameTabMenuWidget;
class UDamageTextComponent;

/**
 * PlayerController that handles movement, camera look, and ability input for RPG characters.
 */
UCLASS()
class RPG_API ARPGPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	/** Enhanced Input contexts - permite múltiplos contextos de input */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;

	/** Movement input action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Look input action (camera rotation) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	void Jump();
	void StopJumping();
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

    

	/** Ability input delegates */
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	/** Data asset for mapping ability inputs to tags */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<URPGInputConfig> InputConfig;

	/** Cached AbilitySystemComponent of the possessed pawn */
	UPROPERTY()
	TObjectPtr<URPGAbilitySystemComponent> RPGAbilitySystemComponent;
	URPGAbilitySystemComponent* GetASC();
	

	// === MENU SYSTEM ===
	
	/** Widget class for the game tab menu */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGameTabMenuWidget> GameTabMenuWidgetClass;
	
	/** Current instance of the game menu */
	UPROPERTY()
	TObjectPtr<UGameTabMenuWidget> CurrentGameMenu;
	
	bool IsAlive() const;
};

