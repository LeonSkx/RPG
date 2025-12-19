#include "UI/Characters/Enemy/EnemyHealthUserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

#include "Components/WidgetComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Character/RPGEnemy.h"


void UEnemyHealthUserWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Não fazer auto-bind aqui - será feito via SetWidgetController
    // que é chamado após a inicialização completa do inimigo
}

void UEnemyHealthUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // Executar decremento gradual da barra de trás se necessário
    if (bIsAnimating)
    {
        TickBackBarAnimation(InDeltaTime);
    }
}

void UEnemyHealthUserWidget::SetEnemyController(ARPGEnemy* InEnemy)
{
    if (!InEnemy)
    {
        return;
    }

    EnemyController = InEnemy;

    if (UAbilitySystemComponent* ASC = EnemyController->GetAbilitySystemComponent())
    {
        // Verificar se o AttributeSet existe
        if (URPGAttributeSet* AttributeSet = Cast<URPGAttributeSet>(EnemyController->GetAttributeSet()))
        {
            // Associa callbacks usando o AttributeValueChangeDelegate do GAS
            ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetHealthAttribute()).AddUObject(this, &UEnemyHealthUserWidget::HealthChanged);
            ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UEnemyHealthUserWidget::MaxHealthChanged);

            // Atualiza variáveis para valores iniciais
            CurrentHealth = ASC->GetNumericAttribute(URPGAttributeSet::GetHealthAttribute());
            MaxHealth = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxHealthAttribute());
            
            // Inicializar sistema de barras duplas
            const float InitialPercent = MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 1.0f;
            CurrentBackPercent = InitialPercent;
            TargetBackPercent = InitialPercent;
            
            // Definir valor inicial da HealthBarBack
            if (HealthBarBack)
            {
                HealthBarBack->SetPercent(CurrentBackPercent);
            }
            
            UpdateHealthUI();
        }
    }
}

void UEnemyHealthUserWidget::HealthChanged(const FOnAttributeChangeData& Data)
{
    CurrentHealth = Data.NewValue;
    UpdateHealthUI();
}

void UEnemyHealthUserWidget::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
    MaxHealth = Data.NewValue;
    UpdateHealthUI();
}

void UEnemyHealthUserWidget::UpdateHealthUI()
{
    // Atualizar barra da frente instantaneamente
    UpdateHealthBarFront();
    
    // Atualizar texto
    if (HealthText)
    {
        FString HealthString = FString::Printf(TEXT("%.0f/%.0f"), CurrentHealth, MaxHealth);
        HealthText->SetText(FText::FromString(HealthString));
    }
    
    // Calcular novo percentual alvo
    const float NewTargetPercent = MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
    
    // Se a vida diminuiu, iniciar decremento gradual da barra de trás
    if (NewTargetPercent < CurrentBackPercent)
    {
        TargetBackPercent = NewTargetPercent;
        StartBackBarAnimation();
    }
    // Se a vida aumentou, atualizar barra de trás instantaneamente
    else if (NewTargetPercent > CurrentBackPercent)
    {
        TargetBackPercent = NewTargetPercent;
        CurrentBackPercent = NewTargetPercent;
        if (HealthBarBack)
        {
            HealthBarBack->SetPercent(CurrentBackPercent);
        }
        
        // Parar qualquer animação em andamento
        bIsAnimating = false;
    }
}

void UEnemyHealthUserWidget::UpdateHealthBarFront()
{
    if (HealthBarFront)
    {
        const float Percent = MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
        HealthBarFront->SetPercent(Percent);
    }
}

void UEnemyHealthUserWidget::StartBackBarAnimation()
{
    // Iniciar decremento gradual imediatamente (sem delay)
    bIsAnimating = true;
}

void UEnemyHealthUserWidget::TickBackBarAnimation(float DeltaTime)
{
    if (bIsAnimating)
    {
        // Processar decremento gradual da barra
        if (!HealthBarBack)
        {
            bIsAnimating = false;
            return;
        }
        
        // Decrementar apenas a cada TickRate para manter 20 FPS
        static float TickAccumulator = 0.0f;
        TickAccumulator += DeltaTime;
        
        if (TickAccumulator >= TickRate)
        {
            TickAccumulator = 0.0f;
            
            // Decrementar 1 em 1 (DecrementStep por tick)
            if (CurrentBackPercent > TargetBackPercent)
            {
                CurrentBackPercent = FMath::Max(CurrentBackPercent - DecrementStep, TargetBackPercent);
                HealthBarBack->SetPercent(CurrentBackPercent);
                
                // Verificar se chegou ao alvo
                if (CurrentBackPercent <= TargetBackPercent)
                {
                    bIsAnimating = false;
                }
            }
            else
            {
                bIsAnimating = false;
            }
        }
    }
} 