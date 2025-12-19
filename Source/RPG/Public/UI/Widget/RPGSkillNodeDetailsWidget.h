// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RPGSkillNodeDetailsWidget.generated.h"

/**
 * Widget que aparece quando o mouse passa sobre um nó da skill tree
 * TESTE SIMPLES - só mostra/oculta
 */
UCLASS()
class RPG_API URPGSkillNodeDetailsWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    // @brief Mostrar o widget
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    void ShowDetails();

    // @brief Ocultar o widget
    UFUNCTION(BlueprintCallable, Category = "SkillTree")
    void HideDetails();

    // @brief Se o widget está visível
    UPROPERTY(BlueprintReadOnly, Category = "SkillTree")
    bool bIsVisible = false;
}; 