#include "UI/Equipment/CharacterSelectorWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Character/RPGCharacter.h"
#include "Character/PlayerClassInfo.h"
#include "Game/RPGGameModeBase.h"
#include "Kismet/GameplayStatics.h"

void UCharacterSelectorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Configurar botão de seleção
	if (SelectButton)
	{
		SelectButton->OnClicked.RemoveAll(this);
		SelectButton->OnClicked.AddDynamic(this, &UCharacterSelectorWidget::HandleSelectButtonClicked);
	}
}

void UCharacterSelectorWidget::Setup(ARPGCharacter* InCharacter, const FString& DisplayName)
{
	CurrentCharacter = InCharacter;
	
	UpdateClassIcon();
}

void UCharacterSelectorWidget::Clear()
{
	CurrentCharacter = nullptr;
	
	// Limpar ícone
	if (ClassIconImage)
	{
		ClassIconImage->SetBrushFromTexture(nullptr);
	}
	
	// Resetar estado visual
	SetSelectedState(false);
}

void UCharacterSelectorWidget::HandleSelectButtonClicked()
{
	if (CurrentCharacter)
	{
		// Disparar delegate para notificar seleção
		OnCharacterSelected.Broadcast(CurrentCharacter);
	}
}

void UCharacterSelectorWidget::UpdateClassIcon()
{
	if (!CurrentCharacter || !ClassIconImage)
	{
		return;
	}

	// Obter GameMode
	ARPGGameModeBase* GameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!GameMode || !GameMode->PlayerClassInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateClassIcon: GameMode ou PlayerClassInfo não encontrado!"));
		return;
	}

	// Obter PlayerClass do personagem
	EPlayerClass PlayerClass = CurrentCharacter->GetPlayerClass();
	
	// Buscar dados da classe
	FPlayerClassData ClassData = GameMode->PlayerClassInfo->GetPlayerClassInfo(PlayerClass);
	
	// Definir ícone
	if (ClassData.ClassIcon)
	{
		ClassIconImage->SetBrushFromTexture(ClassData.ClassIcon);
		UE_LOG(LogTemp, Log, TEXT("UpdateClassIcon: Ícone definido para classe %d do personagem %s"), 
			(int32)PlayerClass, *CurrentCharacter->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateClassIcon: Ícone não encontrado para classe %d do personagem %s"), 
			(int32)PlayerClass, *CurrentCharacter->GetName());
		ClassIconImage->SetBrushFromTexture(nullptr);
	}
}

void UCharacterSelectorWidget::SetSelectedState(bool bIsSelected)
{
	// Atualizar imagem de destaque
	if (SelectionHighlight)
	{
		SelectionHighlight->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	
	// Atualizar escala do container principal (1.5x quando selecionado)
	if (MainContainer)
	{
		FVector2D NewScale = bIsSelected ? FVector2D(2.0f, 2.0f) : FVector2D(1.0f, 1.0f);
		MainContainer->SetRenderScale(NewScale);
	}
}
