#pragma once

#include "CoreMinimal.h"
#include "UI/Characters/BaseOverlayWidget.h"

#include "DefaultOverlayWidget.generated.h"

/**
 * Overlay padrão de gameplay (genérico, sem dependência de personagem)
 */
UCLASS()
class RPG_API UDefaultOverlayWidget : public UBaseOverlayWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
};


