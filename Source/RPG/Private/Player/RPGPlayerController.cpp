#include "Player/RPGPlayerController.h"
#include "CoreMinimal.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "Game/RPGGameModeBase.h"
#include "Input/RPGInputComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

#include "UI/Menus/GameTabMenuWidget.h"
#include "UI/HUD/RPGHUD.h"
#include "Party/PartySubsystem.h"


ARPGPlayerController::ARPGPlayerController()
{
    bReplicates = true;
}

void ARPGPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    // sem lógica por frame
}



void ARPGPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        for (UInputMappingContext* Context : InputMappingContexts)
        {
            if (Context != nullptr)
            {
                Subsystem->AddMappingContext(Context, 0);
            }
        }
    }
    // Define input mode apenas para jogo (captura mouse para rotação)
    FInputModeGameOnly InputModeData;
    SetInputMode(InputModeData);
    // Esconde cursor para third-person view
    bShowMouseCursor = false;
    DefaultMouseCursor = EMouseCursor::Default;
}

void ARPGPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    auto* RPGInput = CastChecked<URPGInputComponent>(InputComponent);
    // Bind de movimento e câmera
    if (MoveAction)
    {
        RPGInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
    }
    if (LookAction)
    {
        RPGInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
    }
    RPGInput->BindAction(ShiftAction, ETriggerEvent::Started, this, &ThisClass::ShiftPressed);
    RPGInput->BindAction(ShiftAction, ETriggerEvent::Completed, this, &ThisClass::ShiftReleased);
    // Bind das ações de habilidade
    RPGInput->BindAbilityActions(InputConfig, this,
        &ThisClass::AbilityInputTagPressed,
        &ThisClass::AbilityInputTagReleased,
        &ThisClass::AbilityInputTagHeld);
}

void ARPGPlayerController::Move(const FInputActionValue& InputValue)
{
    // Bloquear movimento se menu estiver aberto
    if (CurrentGameMenu && CurrentGameMenu->IsInViewport())
    {
        return;
    }
    
    if (auto* ASC = GetASC())
    {
        if (ASC->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputPressed))
            return;
    }
    FVector2D Axis = InputValue.Get<FVector2D>();
    FRotator Rot = GetControlRotation();
    Rot.Pitch = 0.f;
    Rot.Roll = 0.f;
    FVector Forward = FRotationMatrix(Rot).GetUnitAxis(EAxis::X);
    FVector Right   = FRotationMatrix(Rot).GetUnitAxis(EAxis::Y);
    if (APawn* P = GetPawn())
    {
        P->AddMovementInput(Forward, Axis.Y);
        P->AddMovementInput(Right,  Axis.X);
    }
}

void ARPGPlayerController::Look(const FInputActionValue& Value)
{
    const FVector2D V = Value.Get<FVector2D>();
    AddYawInput  (V.X);
    AddPitchInput(V.Y);
}

void ARPGPlayerController::AbilityInputTagPressed(FGameplayTag Tag)
{
    const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
    
    // === HANDLE MENU INPUT TAGS ===
    if (Tag.MatchesTagExact(GameplayTags.InputTag_OpenMenu))
    {
        OpenGameMenu();
        return;
    }
    else if (Tag.MatchesTagExact(GameplayTags.InputTag_CloseMenu))
    {
        CloseGameMenu();
        return;
    }
    else if (Tag.MatchesTagExact(GameplayTags.InputTag_ToggleMenu))
    {
        ToggleGameMenu();
        return;
    }
    
    // === HANDLE PARTY SWITCH TAGS ===
    if (Tag.MatchesTagExact(GameplayTags.InputTag_NextPartyMember))
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                PartySubsystem->SwitchToNextPartyMember();
            }
        }
        return;
    }
    else if (Tag.MatchesTagExact(GameplayTags.InputTag_PreviousPartyMember))
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                PartySubsystem->SwitchToPreviousPartyMember();
            }
        }
        return;
    }
    else if (Tag.MatchesTagExact(GameplayTags.InputTag_PartyMember_1))
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                PartySubsystem->SwitchToPartyMember(0);
            }
        }
        return;
    }
    else if (Tag.MatchesTagExact(GameplayTags.InputTag_PartyMember_2))
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                PartySubsystem->SwitchToPartyMember(1);
            }
        }
        return;
    }
    else if (Tag.MatchesTagExact(GameplayTags.InputTag_PartyMember_3))
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                PartySubsystem->SwitchToPartyMember(2);
            }
        }
        return;
    }
    else if (Tag.MatchesTagExact(GameplayTags.InputTag_PartyMember_4))
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
            {
                PartySubsystem->SwitchToPartyMember(3);
            }
        }
        return;
    }
    
    // === HANDLE ABILITY INPUT TAGS ===
    // Bloquear abilities se menu estiver aberto
    if (CurrentGameMenu && CurrentGameMenu->IsInViewport())
    {
        return;
    }
    
    if (auto* ASC = GetASC())
    {
        const bool bBlocked = ASC->HasMatchingGameplayTag(GameplayTags.Player_Block_InputPressed);
        const bool bComboOwnerActive = ASC->HasActiveAbilityWithInputTag(Tag);
        if (!bBlocked || bComboOwnerActive)
        {
            ASC->AbilityInputTagPressed(Tag);
        }
    }
}

void ARPGPlayerController::AbilityInputTagReleased(FGameplayTag Tag)
{
    // Bloquear abilities se menu estiver aberto
    if (CurrentGameMenu && CurrentGameMenu->IsInViewport())
    {
        return;
    }
    
    if (auto* ASC = GetASC())
    {
        const bool bBlocked = ASC->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputReleased);
        const bool bComboOwnerActive = ASC->HasActiveAbilityWithInputTag(Tag);
        if (!bBlocked || bComboOwnerActive)
        {
            ASC->AbilityInputTagReleased(Tag);
        }
    }
}

void ARPGPlayerController::AbilityInputTagHeld(FGameplayTag Tag)
{
    // Bloquear abilities se menu estiver aberto
    if (CurrentGameMenu && CurrentGameMenu->IsInViewport())
    {
        return;
    }
    
    if (auto* ASC = GetASC())
    {
        const bool bBlocked = ASC->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputHeld);
        const bool bComboOwnerActive = ASC->HasActiveAbilityWithInputTag(Tag);
        if (!bBlocked || bComboOwnerActive)
        {
            // Para combos, só processar Held se a habilidade NÃO estiver ativa (evita duplicação com Pressed)
            if (!bComboOwnerActive)
            {
                ASC->AbilityInputTagHeld(Tag);
            }
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

// === MENU SYSTEM IMPLEMENTATION ===

void ARPGPlayerController::OpenGameMenu()
{
    if (!GameTabMenuWidgetClass)
    {
        return;
    }
    if (CurrentGameMenu && CurrentGameMenu->IsInViewport())
    {
        return;
    }
    CurrentGameMenu = CreateWidget<UGameTabMenuWidget>(this, GameTabMenuWidgetClass);
    if (!CurrentGameMenu) return;
    CurrentGameMenu->AddToViewport();
    // Mostrar mouse e definir input para UI
    bShowMouseCursor = true;
    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(CurrentGameMenu->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
}

void ARPGPlayerController::CloseGameMenu()
{
    if (!CurrentGameMenu || !CurrentGameMenu->IsInViewport())
    {
        return;
    }
    CurrentGameMenu->RemoveFromParent();
    CurrentGameMenu = nullptr;
}

void ARPGPlayerController::ToggleGameMenu()
{
    // Verificar se já existe um menu aberto
    if (CurrentGameMenu && CurrentGameMenu->IsInViewport())
    {
        CloseGameMenu();
    }
    else
    {
        OpenGameMenu();
    }
}

void ARPGPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    // Limpar referência à AbilitySystemComponent ao trocar de personagem
    RPGAbilitySystemComponent = nullptr;
}



// RPCs para troca de membro da party no servidor
void ARPGPlayerController::Server_SetPartySwitchingLocked_Implementation(bool bLocked)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
		{
			PartySubsystem->SetPartySwitchingLocked(bLocked);
		}
	}
}