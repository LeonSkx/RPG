
#include "UI/WidgetController/RPGWidgetController.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Player/RPGPlayerController.h"

void URPGWidgetController::Initialize(APlayerController* InPlayerController, APlayerState* InPlayerState, 
                                      UAbilitySystemComponent* InAbilitySystemComponent, UAttributeSet* InAttributeSet)
{
    PlayerController = InPlayerController;
    PlayerState = InPlayerState;
    AbilitySystemComponent = InAbilitySystemComponent;
    AttributeSet = InAttributeSet;
    
    // Inicializar referências tipadas
    RPGPlayerController = Cast<ARPGPlayerController>(InPlayerController);
    RPGAbilitySystemComponent = Cast<URPGAbilitySystemComponent>(InAbilitySystemComponent);
    RPGAttributeSet = Cast<URPGAttributeSet>(InAttributeSet);
    
    // Conectar callbacks e inicializar valores
    BindCallbacksToDependencies();
    BroadcastInitialValues();
}

void URPGWidgetController::BroadcastInitialValues()
{
    // Implementação base vazia - classes derivadas devem sobrescrever
}

void URPGWidgetController::BindCallbacksToDependencies()
{
    // Implementação base vazia - classes derivadas devem sobrescrever
}

ARPGPlayerController* URPGWidgetController::GetRPGPC() const
{
    return RPGPlayerController;
}

URPGAbilitySystemComponent* URPGWidgetController::GetRPGASC() const
{
    return RPGAbilitySystemComponent;
}

URPGAttributeSet* URPGWidgetController::GetRPGAS() const
{
    return RPGAttributeSet;
} 