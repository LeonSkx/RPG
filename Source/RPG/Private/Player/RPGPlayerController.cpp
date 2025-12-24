// Copyright (c) 2025 RPG Yumi Project. All rights reserved.


#include "Player/RPGPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "Character/RPGCharacterBase.h"
#include "GameFramework/Character.h"
#include "Input/RPGInputComponent.h"
#include "Interaction/CombatInterface.h"

void ARPGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!IsValid(InputSubsystem)) return;

	for (UInputMappingContext* Context : InputMappingContexts)
	{
		InputSubsystem->AddMappingContext(Context, 0);
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!IsValid(EnhancedInputComponent)) return;

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);

	// Sistema de Input - bind das ações de habilidade usando o InputConfig
	if (InputConfig)
	{
		URPGInputComponent* YPInput = Cast<URPGInputComponent>(EnhancedInputComponent);
		if (YPInput)
		{
			YPInput->BindAbilityActions(InputConfig, this,
				&ThisClass::AbilityInputTagPressed,
				&ThisClass::AbilityInputTagReleased,
				&ThisClass::AbilityInputTagHeld);
		}
	}
}

void ARPGPlayerController::Jump()
{
	if (!IsValid(GetCharacter())) return;
	if (!IsAlive()) return;

	GetCharacter()->Jump();
}

void ARPGPlayerController::StopJumping()
{
	if (!IsValid(GetCharacter())) return;
	if (!IsAlive()) return;

	GetCharacter()->StopJumping();
}

void ARPGPlayerController::Move(const FInputActionValue& Value)
{
	if (!IsValid(GetPawn())) return;
	if (!IsAlive()) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();

	// Find which way is forward
	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	GetPawn()->AddMovementInput(ForwardDirection, MovementVector.Y);
	GetPawn()->AddMovementInput(RightDirection, MovementVector.X);
}

void ARPGPlayerController::Look(const FInputActionValue& Value)
{
	if (!IsAlive()) return;
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddYawInput(LookAxisVector.X);
	AddPitchInput(LookAxisVector.Y);
}

// Sistema de Input - métodos para InputConfig
void ARPGPlayerController::AbilityInputTagPressed(FGameplayTag Tag)
{
	if (!IsAlive()) return;
	
	if (URPGAbilitySystemComponent* ASC = GetASC())
	{
		// Verifica se há um combo ativo com este input tag
		// Se houver, permite o input mesmo se houver bloqueio (para combos funcionarem)
		const bool bComboOwnerActive = ASC->HasActiveAbilityWithInputTag(Tag);
		
		// Sempre processa o input - o sistema de combo precisa receber o input mesmo quando ativo
		ASC->AbilityInputTagPressed(Tag);
	}
}

void ARPGPlayerController::AbilityInputTagReleased(FGameplayTag Tag)
{
	if (!IsAlive()) return;
	
	if (URPGAbilitySystemComponent* ASC = GetASC())
	{
		const bool bComboOwnerActive = ASC->HasActiveAbilityWithInputTag(Tag);
		ASC->AbilityInputTagReleased(Tag);
	}
}

void ARPGPlayerController::AbilityInputTagHeld(FGameplayTag Tag)
{
	if (!IsAlive()) return;
	
	if (URPGAbilitySystemComponent* ASC = GetASC())
	{
		const bool bComboOwnerActive = ASC->HasActiveAbilityWithInputTag(Tag);
		if (!bComboOwnerActive)
		{
			ASC->AbilityInputTagHeld(Tag);
		}
	}
}

URPGAbilitySystemComponent* ARPGPlayerController::GetASC()
{
	if (!RPGAbilitySystemComponent)
	{
		RPGAbilitySystemComponent = Cast<URPGAbilitySystemComponent>(
			UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()));
	}
	return RPGAbilitySystemComponent;
}

void ARPGPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RPGAbilitySystemComponent = nullptr;
}

bool ARPGPlayerController::IsAlive() const
{
	APawn* Char = GetPawn();
	if (!IsValid(Char) || !Char->Implements<UCombatInterface>()) return false;
	return !ICombatInterface::Execute_IsDead(Char);
}
