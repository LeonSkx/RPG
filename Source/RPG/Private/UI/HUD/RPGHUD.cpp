#include "UI/HUD/RPGHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/Characters/BaseOverlayWidget.h"
#include "UI/Quest/QuestWidget.h"

ARPGHUD::ARPGHUD()
{
    
    // Inicializar ponteiros como nullptr - Sistema de personagens
    DefaultOverlay = nullptr;
    CurrentOverlay = nullptr;
    MessageWidget = nullptr;
}

void ARPGHUD::BeginPlay()
{
    Super::BeginPlay();

    CreateOverlayWidgets();
    
    // Exibir o overlay padrão no início (sem dependência de personagem)
    if (DefaultOverlay)
    {
        DefaultOverlay->AddToViewport();
        CurrentOverlay = DefaultOverlay;
    }

    // Criar e exibir o QuestWidget sempre visível
    if (UWorld* World = GetWorld())
    {
        if (QuestWidgetClass)
        {
            QuestWidgetInstance = CreateWidget<UQuestWidget>(World, QuestWidgetClass);
            if (QuestWidgetInstance)
            {
                QuestWidgetInstance->AddToViewport();
            }
        }
    }
}

void ARPGHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Limpar overlays
    HideAllOverlays();
    
    // Limpar referências
    CurrentOverlay = nullptr;
    if (QuestWidgetInstance)
    {
        QuestWidgetInstance->RemoveFromParent();
        QuestWidgetInstance = nullptr;
    }
    
    Super::EndPlay(EndPlayReason);
}

// Função template para criar overlays de forma consistente
template<typename TOverlayWidget>
TOverlayWidget* ARPGHUD::CreateOverlayWidget(TSubclassOf<TOverlayWidget> OverlayClass, const FString& CharacterName)
{
    if (!OverlayClass)
    {
        return nullptr;
    }

    if (UWorld* World = GetWorld())
    {
        TOverlayWidget* NewOverlay = CreateWidget<TOverlayWidget>(World, OverlayClass);
        return NewOverlay;
    }
    return nullptr;
}

void ARPGHUD::CreateOverlayWidgets()
{
    UE_LOG(LogTemp, Display, TEXT("RPGHUD: Criando widgets de overlay..."));

    // Usar função template para reduzir código duplicado
    DefaultOverlay = CreateOverlayWidget<UDefaultOverlayWidget>(DefaultOverlayClass, TEXT("Default"));
}

 

void ARPGHUD::HideAllOverlays()
{
	if (DefaultOverlay)
	{
		DefaultOverlay->RemoveFromParent();
	}
	
	CurrentOverlay = nullptr;
}

 

void ARPGHUD::SetGameplayHUDVisibility(bool bVisible)
{
    if (bVisible)
    {
        if (DefaultOverlay && !DefaultOverlay->IsInViewport())
        {
            DefaultOverlay->AddToViewport();
        }
    }
    else
    {
        HideAllOverlays();
    }
}

UUserWidget* ARPGHUD::GetMessageWidget() const
{
    return MessageWidget;
}

void ARPGHUD::ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	if (CurrentOverlay)
	{
		CurrentOverlay->ShowDamageNumber(DamageAmount, TargetCharacter, bBlockedHit, bCriticalHit);
	}
}

void ARPGHUD::InitWidgetControllerSystem(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	// No novo sistema, a inicialização é feita automaticamente via PartySkillsManager
	// Esta função é mantida para compatibilidade mas não faz nada
}





