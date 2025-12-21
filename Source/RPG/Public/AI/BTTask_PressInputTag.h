#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "BTTask_PressInputTag.generated.h"

/**
 * BT Task que simula input por GameplayTag no Ability System (Pressed/Held/Released).
 * - Pressiona no ExecuteTask
 * - Opcionalmente espera HoldTime e solta no Tick
 */
UCLASS()
class RPG_API UBTTask_PressInputTag : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_PressInputTag();

    // Tag de input a ser enviada ao ASC (deve coincidir com StartupInputTag das abilities)
    UPROPERTY(EditAnywhere, Category = "Ability|Input")
    FGameplayTag InputTag;

    // Tempo que o input ficará "segurado" antes de soltar (simula tempo de pressão do dedo)
    // Padrão: 0.05s (50ms) - tempo típico que um dedo fica pressionando um botão
    UPROPERTY(EditAnywhere, Category = "Ability|Input", meta = (ClampMin = "0.0"))
    float HoldTimeSeconds = 0.05f;

    // Número de cliques consecutivos que a task irá emitir (cada ciclo: Press -> [Hold] -> [Release] -> Gap)
    UPROPERTY(EditAnywhere, Category = "Ability|Input", meta = (ClampMin = "1"))
    int32 Clicks = 1;

    // Intervalo entre cliques consecutivos (após Release de cada clique)
    // Padrão: 0.4s (400ms) - tempo típico entre cliques consecutivos de um humano
    UPROPERTY(EditAnywhere, Category = "Ability|Input", meta = (ClampMin = "0.0"))
    float InterClickDelaySeconds = 0.4f;

    // Se true, envia Released ao final de cada clique
    UPROPERTY(EditAnywhere, Category = "Ability|Input")
    bool bReleaseOnFinish = true;

    // Se true, detecta automaticamente quando há combo disponível e reenvia input (útil para combos)
    // Padrão: true - habilita detecção automática de combos
    UPROPERTY(EditAnywhere, Category = "Ability|Input")
    bool bAutoDetectCombo = true;

    // Delay após enviar input antes de verificar se há combo disponível (em segundos)
    // Padrão: 0.15s (150ms) - tempo para evento de combo ser processado e NextComboName ser definido
    UPROPERTY(EditAnywhere, Category = "Ability|Input", meta = (EditCondition = "bAutoDetectCombo", ClampMin = "0.0"))
    float ComboDetectionDelay = 0.15f;

protected:
    struct FTaskMemory
    {
        float Elapsed = 0.f;        // tempo acumulado do estado atual
        bool bPressedSent = false;  // se o Press deste clique já foi enviado
        int32 ClicksDone = 0;       // quantos cliques já foram concluídos
        bool bWaitingForCombo = false; // se está esperando evento de combo
        enum class EState : uint8 { Holding, Releasing, Gap, WaitingForCombo } State = EState::Holding;
    };

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual uint16 GetInstanceMemorySize() const override { return sizeof(FTaskMemory); }
    virtual FString GetStaticDescription() const override;

private:
    class UAbilitySystemComponent* ResolveASC(const UBehaviorTreeComponent& OwnerComp) const;
};


