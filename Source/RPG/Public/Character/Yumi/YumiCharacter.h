#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacter.h"
#include "YumiCharacter.generated.h"

// Forward declare timer handle
struct FTimerHandle;

/**
 * Yumi character class inheriting from ARPGCharacter.
 */
UCLASS()
class RPG_API AYumiCharacter : public ARPGCharacter
{
    GENERATED_BODY()

public:
    AYumiCharacter(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void BeginPlay() override;

    // Método para logar atributos após inicialização do GAS
    void LogAttributes();

    // Timer handle para adiar o log até atributos carregarem
    FTimerHandle AttributeLogTimerHandle;

public:
    virtual void Tick(float DeltaTime) override;
}; 