#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RPGWidgetController.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
class ARPGPlayerController;
class URPGAbilitySystemComponent;
class URPGAttributeSet;

/**
 * Classe base para todos os widget controllers no jogo
 */
UCLASS()
class RPG_API URPGWidgetController : public UObject
{
    GENERATED_BODY()
    
public:
    /** Inicializa o controller com as referências necessárias */
    virtual void Initialize(APlayerController* InPlayerController, APlayerState* InPlayerState, 
                           UAbilitySystemComponent* InAbilitySystemComponent, UAttributeSet* InAttributeSet);
    
    /** Transmite os valores iniciais para widgets */
    UFUNCTION(BlueprintCallable, Category = "Widget Controller")
    virtual void BroadcastInitialValues();
    
    /** Conecta callbacks aos delegados dependentes */
    virtual void BindCallbacksToDependencies();
    
protected:
    /** Referências às classes fundamentais do jogo */
    UPROPERTY(BlueprintReadOnly, Category = "Widget Controller")
    TObjectPtr<APlayerController> PlayerController;
    
    UPROPERTY(BlueprintReadOnly, Category = "Widget Controller")
    TObjectPtr<APlayerState> PlayerState;
    
    UPROPERTY(BlueprintReadOnly, Category = "Widget Controller")
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
    
    UPROPERTY(BlueprintReadOnly, Category = "Widget Controller")
    TObjectPtr<UAttributeSet> AttributeSet;
    
    /** Referências tipadas para componentes específicos do RPG */
    UPROPERTY(BlueprintReadOnly, Category = "Widget Controller")
    TObjectPtr<ARPGPlayerController> RPGPlayerController;
    
    UPROPERTY(BlueprintReadOnly, Category = "Widget Controller")
    TObjectPtr<URPGAbilitySystemComponent> RPGAbilitySystemComponent;
    
    UPROPERTY(BlueprintReadOnly, Category = "Widget Controller")
    TObjectPtr<URPGAttributeSet> RPGAttributeSet;
    
    /** Métodos auxiliares para obter referências tipadas */
    ARPGPlayerController* GetRPGPC() const;
    URPGAbilitySystemComponent* GetRPGASC() const;
    URPGAttributeSet* GetRPGAS() const;
}; 