#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/RPGUserWidget.h"
#include "Character/RPGCharacter.h"
#include "CharacterSelectorWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;
class USizeBox;

// Delegate para quando um personagem é selecionado
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelected, ARPGCharacter*, SelectedCharacter);

/**
 * Widget simples para seleção de personagens da party
 * Exibe um botão com nome do personagem
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API UCharacterSelectorWidget : public URPGUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	/** Configura o widget com um personagem específico */
	UFUNCTION(BlueprintCallable, Category = "Character Selector")
	void Setup(ARPGCharacter* InCharacter, const FString& DisplayName);

	/** Limpa os dados do widget */
	UFUNCTION(BlueprintCallable, Category = "Character Selector")
	void Clear();

	/** Verifica se tem personagem válido */
	UFUNCTION(BlueprintPure, Category = "Character Selector")
	bool HasCharacter() const { return CurrentCharacter != nullptr; }

	/** Obtém o personagem atual */
	UFUNCTION(BlueprintPure, Category = "Character Selector")
	ARPGCharacter* GetCurrentCharacter() const { return CurrentCharacter; }

	/** Define o estado de seleção do widget */
	UFUNCTION(BlueprintCallable, Category = "Character Selector")
	void SetSelectedState(bool bIsSelected);

	/** Delegate para quando personagem é selecionado */
	UPROPERTY(BlueprintAssignable, Category = "Character Selector")
	FOnCharacterSelected OnCharacterSelected;

protected:
	/** Referência do personagem atualmente configurado */
	UPROPERTY(BlueprintReadOnly, Category = "Character Selector")
	ARPGCharacter* CurrentCharacter = nullptr;

	/** Botão principal do seletor */
	UPROPERTY(meta = (BindWidget))
	UButton* SelectButton = nullptr;



	/** Imagem do ícone da classe do personagem */
	UPROPERTY(meta = (BindWidget))
	UImage* ClassIconImage = nullptr;

	/** Imagem de destaque quando selecionado */
	UPROPERTY(meta = (BindWidget))
	UImage* SelectionHighlight = nullptr;

	/** Container principal para controle de escala */
	UPROPERTY(meta = (BindWidget))
	USizeBox* MainContainer = nullptr;

private:
	/** Handler para clique no botão de seleção */
	UFUNCTION()
	void HandleSelectButtonClicked();

	/** Atualiza o ícone da classe baseado no personagem atual */
	void UpdateClassIcon();
};
