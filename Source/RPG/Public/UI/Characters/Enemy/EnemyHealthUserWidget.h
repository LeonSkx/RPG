#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "UI/Widget/RPGUserWidget.h"
#include "EnemyHealthUserWidget.generated.h"

class ARPGEnemy;

UCLASS()
class RPG_API UEnemyHealthUserWidget : public URPGUserWidget
{
    GENERATED_BODY()

public:
    // Define o controlador (inimigo) que fornece dados de vida
    UFUNCTION(BlueprintCallable, Category = "Enemy UI")
    void SetEnemyController(ARPGEnemy* InEnemy);

protected:
    virtual void NativeConstruct() override;

    // Bind widgets do UMG Designer: os nomes abaixo devem corresponder aos nomes dos widgets no BP
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UProgressBar* HealthBarFront;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UProgressBar* HealthBarBack;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* HealthText;

private:
    // Referência ao inimigo para obter valores
    UPROPERTY()
    ARPGEnemy* EnemyController;

    // Valores atuais de vida para UI
    float CurrentHealth = 0.0f;
    float MaxHealth = 0.0f;
    
    // Sistema de animação da barra de trás
    float TargetBackPercent = 1.0f;    // Percentual alvo para HealthBarBack
    float CurrentBackPercent = 1.0f;   // Percentual atual de HealthBarBack
    float DecrementStep = 0.01f;       // Quanto decrementa por tick (1% = 0.01)
    
    // Sistema de tick para animação suave
    float TickRate = 0.05f;            // 20 FPS (1/20 = 0.05s)
    bool bIsAnimating = false;         // Flag se está animando

    // Atualiza os elementos da UI (barra e texto)
    void UpdateHealthUI();
    
    // Funções para sistema de barras animadas
    void UpdateHealthBarFront();
    void StartBackBarAnimation();
    
    // Sistema de tick para animação
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    void TickBackBarAnimation(float DeltaTime);

    // Callbacks para atualizar a UI quando os atributos de vida mudam
    void HealthChanged(const struct FOnAttributeChangeData& Data);
    void MaxHealthChanged(const struct FOnAttributeChangeData& Data);
}; 