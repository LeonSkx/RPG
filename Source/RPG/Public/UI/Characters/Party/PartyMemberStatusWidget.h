#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"

#include "UI/Widget/RPGUserWidget.h"
#include "PartyMemberStatusWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class ARPGCharacter;

/**
 * Widget para mostrar o mini status de um membro do grupo
 */
UCLASS()
class RPG_API UPartyMemberStatusWidget : public URPGUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // Configura o widget com o personagem
    UFUNCTION(BlueprintCallable, Category = "Party")
    void SetupMember(ARPGCharacter* Character);

    // Torna pública para ser chamada pelo Overlay ao resetar/esconder
    void UnbindDelegates();

protected:
    // Barra de vida do membro
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthBar;

    // Nome do membro
    UPROPERTY(meta = (BindWidget))
    UTextBlock* NameText;

    // Texto de vida
    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    // Ícone da classe do personagem
    UPROPERTY(meta = (BindWidget))
    UImage* ClassIcon;

    // Referência ao personagem
    UPROPERTY()
    TWeakObjectPtr<ARPGCharacter> MemberCharacter;

    // Handler para mudança de vida
    UFUNCTION()
    void OnHealthChanged(float NewHealth, float MaxHealth);

    // Handler para mudanças de vida via GAS
    void OnHealthChangedFromGAS(const FOnAttributeChangeData& Data);

    // Handler para mudanças de classe
    UFUNCTION()
    void OnPlayerClassChanged(EPlayerClass NewClass);

private:
    // Funções de atualização de UI
    void UpdateHealth(float Current, float Max);
    void UpdateName(const FString& CharacterName);
    void UpdateClassIcon();
}; 
