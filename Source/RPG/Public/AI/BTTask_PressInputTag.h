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

    // Tempo que o input ficará "segurado" antes de soltar. 0 = solta imediatamente ou conforme bReleaseOnFinish
    UPROPERTY(EditAnywhere, Category = "Ability|Input", meta = (ClampMin = "0.0"))
    float HoldTimeSeconds = 0.f;

    // Número de cliques consecutivos que a task irá emitir (cada ciclo: Press -> [Hold] -> [Release] -> Gap)
    UPROPERTY(EditAnywhere, Category = "Ability|Input", meta = (ClampMin = "1"))
    int32 Clicks = 1;

    // Intervalo entre cliques consecutivos (após Release de cada clique)
    UPROPERTY(EditAnywhere, Category = "Ability|Input", meta = (ClampMin = "0.0"))
    float InterClickDelaySeconds = 0.1f;

    // Se true, envia Released ao final de cada clique
    UPROPERTY(EditAnywhere, Category = "Ability|Input")
    bool bReleaseOnFinish = true;

protected:
    struct FTaskMemory
    {
        float Elapsed = 0.f;        // tempo acumulado do estado atual
        bool bPressedSent = false;  // se o Press deste clique já foi enviado
        int32 ClicksDone = 0;       // quantos cliques já foram concluídos
        enum class EState : uint8 { Holding, Releasing, Gap } State = EState::Holding;
    };

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual uint16 GetInstanceMemorySize() const override { return sizeof(FTaskMemory); }
    virtual FString GetStaticDescription() const override;

private:
    class UAbilitySystemComponent* ResolveASC(const UBehaviorTreeComponent& OwnerComp) const;
};


