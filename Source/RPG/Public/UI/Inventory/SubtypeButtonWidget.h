#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/RPGUserWidget.h"
#include "Inventory/Core/InventoryEnums.h"
#include "SubtypeButtonWidget.generated.h"

class UButton;
class UImage;
class UTexture2D;
class USizeBox;

UCLASS(BlueprintType, Blueprintable)
class RPG_API USubtypeButtonWidget : public URPGUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    /** Configura o botão com dados do subtype */
    UFUNCTION(BlueprintCallable, Category = "Subtype Button")
    void SetupSubtype(const FString& InSubtypeName, UTexture2D* InIcon = nullptr);

    /** Define se o botão está selecionado */
    UFUNCTION(BlueprintCallable, Category = "Subtype Button")
    void SetSelected(bool bInSelected);

    /** Verifica se o botão está selecionado */
    UFUNCTION(BlueprintPure, Category = "Subtype Button")
    bool IsSelected() const { return bIsSelected; }

    /** Obtém o nome do subtype */
    UFUNCTION(BlueprintPure, Category = "Subtype Button")
    FString GetSubtypeName() const { return SubtypeName; }

    /** Callback disparado quando o botão é clicado */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubtypeClicked, const FString&, SubtypeName);
    UPROPERTY(BlueprintAssignable, Category = "Subtype Button")
    FOnSubtypeClicked OnSubtypeClicked;

    /** Define o ícone do subtype */
    UFUNCTION(BlueprintCallable, Category = "Subtype Button")
    void SetSubtypeIcon(UTexture2D* InIcon);

protected:
    /** Callback para quando o botão é clicado */
    UFUNCTION()
    void HandleClicked();

    /** Nome do subtype representado por este botão */
    UPROPERTY(BlueprintReadOnly, Category = "Subtype Button")
    FString SubtypeName;

    /** Flag indicando se o botão está selecionado */
    UPROPERTY(BlueprintReadOnly, Category = "Subtype Button", meta = (AllowPrivateAccess = "true"))
    bool bIsSelected = false;

    /** Container principal do botão */
    UPROPERTY(meta = (BindWidget))
    USizeBox* MainContainer = nullptr;

    /** Botão clicável */
    UPROPERTY(meta = (BindWidget))
    UButton* SubtypeButton = nullptr;

    /** Ícone do subtype */
    UPROPERTY(meta = (BindWidget))
    UImage* SubtypeIcon = nullptr;
};
