// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/Characters/BaseOverlayWidget.h"
#include "UI/Characters/Characters/DefaultOverlayWidget.h"
#include "RPGHUD.generated.h"

class UUserWidget;
class UQuestWidget;
/**
 * 
 */
UCLASS()
class RPG_API ARPGHUD : public AHUD
{
	GENERATED_BODY()
public:

	ARPGHUD();

	// Override do ciclo de vida
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	
	// Sistema de Overlays por Personagem
	void HideAllOverlays();
	void SetGameplayHUDVisibility(bool bVisible);

	// Sistema de Damage Numbers
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);

	// Utilitários
	UUserWidget* GetMessageWidget() const;

	// Funções de compatibilidade (mantida para não quebrar código existente)
	void InitWidgetControllerSystem(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

protected:

	// Template function para criação de overlays
	template<typename TOverlayWidget>
	TOverlayWidget* CreateOverlayWidget(TSubclassOf<TOverlayWidget> OverlayClass, const FString& CharacterName);

	// Sistema de criação e gerenciamento
	void CreateOverlayWidgets();

	// Classes dos Overlays Específicos
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Overlays")
	TSubclassOf<UDefaultOverlayWidget> DefaultOverlayClass;


private:


	// Sistema de Overlays por Personagem
	UPROPERTY()
	TObjectPtr<UDefaultOverlayWidget> DefaultOverlay;

	UPROPERTY()
	TObjectPtr<UBaseOverlayWidget> CurrentOverlay;

	// Message Widget para compatibilidade
	UPROPERTY()
	TObjectPtr<UUserWidget> MessageWidget;

    // Quest Widget sempre visível (gerenciado pelo HUD)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<UQuestWidget> QuestWidgetClass;

    UPROPERTY()
    TObjectPtr<UQuestWidget> QuestWidgetInstance;
};
