#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestWidget.generated.h"

class UTextBlock;
class UQuestSubsystem;
class UPartySubsystem;
class ARPGCharacter;

/**
 * Widget simples para exibir a quantidade de quests ativas do personagem atual.
 * Gerenciado pelo RPGHUD e sempre visível.
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API UQuestWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** Atualiza o texto com a contagem de quests ativas. */
    UFUNCTION(BlueprintCallable, Category = "Quest Widget")
    void UpdateActiveQuestCount();

protected:
    // Removido: contador de quests visível

private:
    UPROPERTY()
    UQuestSubsystem* QuestSubsystem = nullptr;

    UPROPERTY()
    UPartySubsystem* PartySubsystem = nullptr;

    /** Handlers dos eventos de quest para manter o texto atualizado. */
    UFUNCTION()
    void HandleQuestAccepted(const FString& QuestID);

    UFUNCTION()
    void HandleQuestCompleted(const FString& QuestID);

    UFUNCTION()
    void HandleQuestFailed(const FString& QuestID);

protected:
    /** Texto exibindo o nome da primeira quest ativa (opcional). */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ActiveQuestNameText;

    /** Texto exibindo o tipo da primeira quest ativa (opcional). */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ActiveQuestTypeText;
};

