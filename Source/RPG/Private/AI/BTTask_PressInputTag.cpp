#include "AI/BTTask_PressInputTag.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "AIController.h"

UBTTask_PressInputTag::UBTTask_PressInputTag()
{
    NodeName = TEXT("Press Input Tag");
    bNotifyTick = true;
}

UAbilitySystemComponent* UBTTask_PressInputTag::ResolveASC(const UBehaviorTreeComponent& OwnerComp) const
{
    const AAIController* AI = OwnerComp.GetAIOwner();
    if (!AI) return nullptr;
    APawn* Pawn = AI->GetPawn();
    if (!Pawn) return nullptr;
    if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
    {
        return ASI->GetAbilitySystemComponent();
    }
    return nullptr;
}

EBTNodeResult::Type UBTTask_PressInputTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FTaskMemory* Mem = (FTaskMemory*)NodeMemory;
    Mem->Elapsed = 0.f;
    Mem->bPressedSent = false;
    Mem->ClicksDone = 0;
    Mem->State = FTaskMemory::EState::Holding;

    if (!InputTag.IsValid())
    {
        return EBTNodeResult::Failed;
    }

    if (URPGAbilitySystemComponent* ASC = Cast<URPGAbilitySystemComponent>(ResolveASC(OwnerComp)))
    {
        // Iniciar primeiro clique
        ASC->AbilityInputTagPressed(InputTag);
        Mem->bPressedSent = true;
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}

void UBTTask_PressInputTag::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FTaskMemory* Mem = (FTaskMemory*)NodeMemory;
    if (!Mem->bPressedSent)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    Mem->Elapsed += DeltaSeconds;

    switch (Mem->State)
    {
    case FTaskMemory::EState::Holding:
        if (HoldTimeSeconds <= 0.f || Mem->Elapsed >= HoldTimeSeconds)
        {
            // Transita para Releasing para dar tempo à habilidade processar
            if (bReleaseOnFinish)
            {
                if (URPGAbilitySystemComponent* ASC = Cast<URPGAbilitySystemComponent>(ResolveASC(OwnerComp)))
                {
                    ASC->AbilityInputTagReleased(InputTag);
                }
            }
            Mem->State = FTaskMemory::EState::Releasing;
            Mem->Elapsed = 0.f;
        }
        break;
    case FTaskMemory::EState::Releasing:
        // Verificar se a habilidade realmente terminou antes de continuar
        if (URPGAbilitySystemComponent* ASC = Cast<URPGAbilitySystemComponent>(ResolveASC(OwnerComp)))
        {
            if (!ASC->HasActiveAbilityWithInputTag(InputTag))
            {
                // Habilidade terminou, pode continuar
                Mem->State = FTaskMemory::EState::Gap;
                Mem->Elapsed = 0.f;
            }
            else if (Mem->Elapsed >= 1.0f) // Timeout de 1 segundo para evitar travamento
            {
                // Timeout - forçar continuação mesmo se habilidade não terminou
                Mem->State = FTaskMemory::EState::Gap;
                Mem->Elapsed = 0.f;
            }
        }
        else
        {
            // ASC não encontrado, continuar
            Mem->State = FTaskMemory::EState::Gap;
            Mem->Elapsed = 0.f;
        }
        break;
    case FTaskMemory::EState::Gap:
        if (Mem->Elapsed >= InterClickDelaySeconds)
        {
            Mem->ClicksDone += 1;
            if (Mem->ClicksDone >= FMath::Max(1, Clicks))
            {
                FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
                return;
            }
            // Iniciar próximo clique
            if (URPGAbilitySystemComponent* ASC = Cast<URPGAbilitySystemComponent>(ResolveASC(OwnerComp)))
            {
                ASC->AbilityInputTagPressed(InputTag);
            }
            Mem->State = FTaskMemory::EState::Holding;
            Mem->Elapsed = 0.f;
        }
        break;
    default:
        break;
    }
}

FString UBTTask_PressInputTag::GetStaticDescription() const
{
    FString TagText = InputTag.IsValid() ? InputTag.ToString() : TEXT("<None>");
    return FString::Printf(TEXT("Press %s, Hold %.2fs, Release %s"), *TagText, HoldTimeSeconds, bReleaseOnFinish ? TEXT("Yes") : TEXT("No"));
}


