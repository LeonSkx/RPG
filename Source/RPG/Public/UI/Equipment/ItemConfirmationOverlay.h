#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "ItemConfirmationOverlay.generated.h"

class UTextBlock;
class UButton;
class ARPGCharacter;

/**
 * Overlay de confirmação para itens compartilhados entre personagens
 * Exibe quando um personagem tenta equipar um item já usado por outro
 */
UCLASS()
class RPG_API UItemConfirmationOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Configura o overlay com informações do conflito */
	UFUNCTION(BlueprintCallable, Category = "Item Confirmation")
	void SetupConfirmation(ARPGCharacter* CurrentUser, ARPGCharacter* TargetCharacter, UItemDataAsset* ItemData);

	/** Exibe o overlay */
	UFUNCTION(BlueprintCallable, Category = "Item Confirmation")
	void ShowOverlay();

	/** Oculta o overlay */
	UFUNCTION(BlueprintCallable, Category = "Item Confirmation")
	void HideOverlay();

protected:
	virtual void NativeConstruct() override;

private:
	/** Configura os eventos dos botões */
	void SetupButtonEvents();

	/** Atualiza o texto de confirmação */
	void UpdateConfirmationText();

	/** Botão Sim - confirma a troca de itens */
	UFUNCTION()
	void OnConfirmClicked();

	/** Botão Não - cancela a operação */
	UFUNCTION()
	void OnCancelClicked();

	/** Executa a troca de itens entre personagens */
	void ExecuteItemSwap();

protected:
	/** Texto explicativo da confirmação */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ConfirmationText = nullptr;

	/** Botão para confirmar a troca */
	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmButton = nullptr;

	/** Botão para cancelar a operação */
	UPROPERTY(meta = (BindWidget))
	UButton* CancelButton = nullptr;

private:
	/** Personagem que atualmente usa o item */
	ARPGCharacter* CurrentItemUser = nullptr;

	/** Personagem que quer equipar o item */
	ARPGCharacter* TargetCharacter = nullptr;

	/** Item que está sendo disputado */
	UItemDataAsset* DisputedItem = nullptr;

	/** Delegate para notificar quando a confirmação é resolvida */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnConfirmationResolved, bool, bConfirmed, ARPGCharacter*, NewUser, ARPGCharacter*, PreviousUser);
	
public:
	/** Evento disparado quando a confirmação é resolvida */
	UPROPERTY(BlueprintAssignable, Category = "Item Confirmation")
	FOnConfirmationResolved OnConfirmationResolved;
};
