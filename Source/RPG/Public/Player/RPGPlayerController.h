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

public:
	ARPGPlayerController();
	virtual void PlayerTick(float DeltaTime) override;



protected:
	virtual void BeginPlay() override;
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

	/** Shift (e.g. sprint) input action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ShiftAction;

	/** Handle movement input */
	void Move(const FInputActionValue& InputActionValue);

	/** Handle look (camera) input */
	void Look(const FInputActionValue& InputActionValue);

	void ShiftPressed()  { bShiftKeyDown = true; }
	void ShiftReleased() { bShiftKeyDown = false; }
	bool bShiftKeyDown = false;

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
	
	/** Opens the game menu with tag validation */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenGameMenu();
	
	/** Closes the game menu with tag validation */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseGameMenu();
	
	/** Toggles the game menu (open/close) */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleGameMenu();

public:
	UFUNCTION(Server, Reliable)
	void Server_SetPartySwitchingLocked(bool bLocked);
};

