#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerClassInfo.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EPlayerClass : uint8
{
    Elementalist, // 🧙‍♂️ Mago
    Guardian,     // 🛡️ Tank
    Scout,        // 🏹 Arqueiro
    Automaton     // 🤖 Robô
};

USTRUCT(BlueprintType)
struct FPlayerClassData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Player Class")
    UTexture2D* ClassIcon = nullptr;
};

UCLASS()
class RPG_API UPlayerClassInfo : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Player Classes")
    TMap<EPlayerClass, FPlayerClassData> PlayerClasses;

    FPlayerClassData GetPlayerClassInfo(EPlayerClass PlayerClass);
};
