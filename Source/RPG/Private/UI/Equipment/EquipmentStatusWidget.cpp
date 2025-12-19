#include "UI/Equipment/EquipmentStatusWidget.h"
#include "Character/RPGCharacterBase.h"
#include "Character/RPGCharacter.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "Inventory/Equipment/EquipmentComparisonComponent.h"
#include "Components/TextBlock.h"

UEquipmentStatusWidget::UEquipmentStatusWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , TargetCharacter(nullptr)
{
    // Construtor vazio
}

void UEquipmentStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // TODO: Implementar inicialização
}

void UEquipmentStatusWidget::SetTargetCharacter(ACharacter* NewTargetCharacter)
{
    TargetCharacter = NewTargetCharacter;
    
    // Atualiza exibição quando personagem muda
    if (TargetCharacter)
    {
        UpdateDisplay();
    }
}

void UEquipmentStatusWidget::UpdateDisplay()
{
    if (!TargetCharacter)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: Nenhum personagem definido"));
        return;
    }

    // Obter o componente de comparação
    if (ARPGCharacter* RPGChar = Cast<ARPGCharacter>(TargetCharacter))
    {
        ComparisonComponent = RPGChar->GetEquipmentComparisonComponent();
    }

    if (!ComparisonComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: ComparisonComponent não encontrado"));
        return;
    }

    // Atualiza todos os 5 atributos core usando o componente de comparação
    UpdateTotalDamageText();  // Ataque
    UpdateTotalArmorText();   // Armadura
    UpdateTotalMagicDamageText(); // Dano Mágico
    UpdateTotalAccuracyText();    // Precisão
    UpdateTotalMagicResistanceText(); // Resistência Mágica
    
    // Atualiza valores simulados
    UpdateSimulatedValues();
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: Atualizando exibição para personagem %s"), 
        *TargetCharacter->GetName());
}

float UEquipmentStatusWidget::GetTotalDamage()
{
    if (!ComparisonComponent)
    {
        return 0.f;
    }

    // Usa o componente de comparação para obter o ataque atual
    return ComparisonComponent->GetCurrentAttack();
}

float UEquipmentStatusWidget::GetAttributeValue(FGameplayAttribute Attribute)
{
    if (!ComparisonComponent)
    {
        return 0.f;
    }

    // Usa o componente de comparação para obter valores
    if (Attribute == URPGAttributeSet::GetArmorAttribute())
    {
        return ComparisonComponent->GetCurrentArmor();
    }
    else if (Attribute == URPGAttributeSet::GetMagicDamageAttribute())
    {
        return ComparisonComponent->GetCurrentMagicDamage();
    }
    else if (Attribute == URPGAttributeSet::GetAccuracyAttribute())
    {
        return ComparisonComponent->GetCurrentAccuracy();
    }
    else if (Attribute == URPGAttributeSet::GetMagicResistanceAttribute())
    {
        return ComparisonComponent->GetCurrentMagicResistance();
    }
    
    return 0.f;
}

float UEquipmentStatusWidget::GetTotalArmor()
{
    // Usa a função genérica para pegar armadura
    return GetAttributeValue(URPGAttributeSet::GetArmorAttribute());
}

float UEquipmentStatusWidget::GetTotalMagicDamage()
{
    // Usa a função genérica para pegar dano mágico
    return GetAttributeValue(URPGAttributeSet::GetMagicDamageAttribute());
}

float UEquipmentStatusWidget::GetTotalAccuracy()
{
    // Usa a função genérica para pegar precisão
    return GetAttributeValue(URPGAttributeSet::GetAccuracyAttribute());
}

float UEquipmentStatusWidget::GetTotalMagicResistance()
{
    // Usa a função genérica para pegar resistência mágica
    return GetAttributeValue(URPGAttributeSet::GetMagicResistanceAttribute());
}

void UEquipmentStatusWidget::UpdateTotalDamageText()
{
    if (!CurrentDamageText)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: CurrentDamageText não encontrado"));
        return;
    }

    // Pega o ataque total
    float TotalDamage = GetTotalDamage();
    
    // TRUNCA o valor (remove decimais sem arredondar)
    int32 TruncatedDamage = FMath::FloorToInt(TotalDamage);
    
    // Formata o texto
    FString DamageText = FString::Printf(TEXT("%d"), TruncatedDamage);
    
    // Atualiza o TextBlock
    CurrentDamageText->SetText(FText::FromString(DamageText));
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: TextBlock atualizado com: %s"), *DamageText);
}

void UEquipmentStatusWidget::UpdateTotalArmorText()
{
    // Usa a função genérica para atualizar armadura
    UpdateAttributeText(CurrentArmorText, URPGAttributeSet::GetArmorAttribute(), TEXT("Armadura"));
}

void UEquipmentStatusWidget::UpdateTotalMagicDamageText()
{
    // Usa a função genérica para atualizar dano mágico
    UpdateAttributeText(CurrentMagicDamageText, URPGAttributeSet::GetMagicDamageAttribute(), TEXT("Dano Mágico"));
}

void UEquipmentStatusWidget::UpdateTotalAccuracyText()
{
    // Usa a função genérica para atualizar precisão
    UpdateAttributeText(CurrentAccuracyText, URPGAttributeSet::GetAccuracyAttribute(), TEXT("Precisão"));
}

void UEquipmentStatusWidget::UpdateTotalMagicResistanceText()
{
    // Usa a função genérica para atualizar resistência mágica
    UpdateAttributeText(CurrentMagicResistanceText, URPGAttributeSet::GetMagicResistanceAttribute(), TEXT("Resistência Mágica"));
}

void UEquipmentStatusWidget::UpdateAttributeText(UTextBlock* TextWidget, FGameplayAttribute Attribute, const FString& Label)
{
    if (!TextWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: TextWidget não encontrado para %s"), *Label);
        return;
    }

    // Pega o valor do atributo
    float AttributeValue = GetAttributeValue(Attribute);
    
    // TRUNCA o valor (remove decimais sem arredondar)
    int32 TruncatedValue = FMath::FloorToInt(AttributeValue);
    
    // Formata o texto
    FString AttributeText = FString::Printf(TEXT("%d"), TruncatedValue);
    
    // Atualiza o TextBlock
    TextWidget->SetText(FText::FromString(AttributeText));
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: TextBlock de %s atualizado com: %s"), *Label, *AttributeText);
}

// === FUNÇÕES PARA VALORES SIMULADOS ===

void UEquipmentStatusWidget::UpdateSimulatedValues()
{
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: === INICIANDO UPDATE SIMULATED VALUES ==="));
    
    if (!ComparisonComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: ComparisonComponent não encontrado"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: ComparisonComponent encontrado - verificando simulações ativas"));
    
    // Verificar se há simulações ativas
    bool HasAnySimulation = ComparisonComponent->HasAnySimulation();
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: HasAnySimulation = %s"), HasAnySimulation ? TEXT("SIM") : TEXT("NÃO"));
    
    if (!HasAnySimulation)
    {
        UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: Nenhuma simulação ativa - valores simulados = valores reais"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: Simulação ativa - atualizando valores simulados"));
    }

    // Atualiza todos os valores simulados
    UpdateSimulatedAtaqueText();
    UpdateSimulatedArmaduraText();
    UpdateSimulatedDanoMagicoText();
    UpdateSimulatedPrecisaoText();
    UpdateSimulatedResistenciaMagicaText();
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: === UPDATE SIMULATED VALUES CONCLUÍDO ==="));
}

void UEquipmentStatusWidget::UpdateSimulatedAtaqueText()
{
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: --- Atualizando SimulateAtaqueText ---"));
    
    if (!SimulateAtaqueText)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: SimulateAtaqueText não encontrado"));
        return;
    }

    if (!ComparisonComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: ComparisonComponent não encontrado em UpdateSimulatedAtaqueText"));
        return;
    }

    // Verifica se há simulação ativa
    if (ComparisonComponent->HasAnySimulation())
    {
        // Pega o texto de comparação do componente
        FString ComparisonText = ComparisonComponent->GetAttackComparisonText();
        UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: Texto de comparação obtido: '%s'"), *ComparisonText);
        
        // Atualiza o TextBlock
        SimulateAtaqueText->SetText(FText::FromString(ComparisonText));
        
        // Aplica cor baseada na simulação
        float CurrentAttack = ComparisonComponent->GetCurrentAttack();
        float SimulatedAttack = ComparisonComponent->GetSimulatedAttack();
        float Delta = SimulatedAttack - CurrentAttack;
        
        if (FMath::IsNearlyZero(Delta, 0.1f)) // Se valores são iguais
        {
            // Oculta o texto quando não há mudança
            SimulateAtaqueText->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            // Mostra o texto com cor apropriada
            if (Delta > 0)
                SimulateAtaqueText->SetColorAndOpacity(FLinearColor::Green);
            else
                SimulateAtaqueText->SetColorAndOpacity(FLinearColor::Red);
            
            // Torna visível
            SimulateAtaqueText->SetVisibility(ESlateVisibility::Visible);
        }
    }
    else
    {
        // Oculta o texto quando não há simulação
        SimulateAtaqueText->SetVisibility(ESlateVisibility::Hidden);
    }
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: SimulateAtaqueText atualizado"));
}

void UEquipmentStatusWidget::UpdateSimulatedArmaduraText()
{
    if (!SimulateArmaduraText)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: SimulateArmaduraText não encontrado"));
        return;
    }

    if (!ComparisonComponent)
    {
        return;
    }

    // Verifica se há simulação ativa
    if (ComparisonComponent->HasAnySimulation())
    {
        // Pega o texto de comparação do componente
        FString ComparisonText = ComparisonComponent->GetArmorComparisonText();
        
        // Atualiza o TextBlock
        SimulateArmaduraText->SetText(FText::FromString(ComparisonText));
        
        // Aplica cor baseada na simulação
        float CurrentArmor = ComparisonComponent->GetCurrentArmor();
        float SimulatedArmor = ComparisonComponent->GetSimulatedArmor();
        float Delta = SimulatedArmor - CurrentArmor;
        
        if (FMath::IsNearlyZero(Delta, 0.1f)) // Se valores são iguais
        {
            // Oculta o texto quando não há mudança
            SimulateArmaduraText->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            // Mostra o texto com cor apropriada
            if (Delta > 0)
                SimulateArmaduraText->SetColorAndOpacity(FLinearColor::Green);
            else
                SimulateArmaduraText->SetColorAndOpacity(FLinearColor::Red);
            
            // Torna visível
            SimulateArmaduraText->SetVisibility(ESlateVisibility::Visible);
        }
    }
    else
    {
        // Oculta o texto quando não há simulação
        SimulateArmaduraText->SetVisibility(ESlateVisibility::Hidden);
    }
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: SimulateArmaduraText atualizado"));
}

void UEquipmentStatusWidget::UpdateSimulatedDanoMagicoText()
{
    if (!SimulateDanoMagicoText)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: SimulateDanoMagicoText não encontrado"));
        return;
    }

    if (!ComparisonComponent)
    {
        return;
    }

    // Verifica se há simulação ativa
    if (ComparisonComponent->HasAnySimulation())
    {
        // Pega o texto de comparação do componente
        FString ComparisonText = ComparisonComponent->GetMagicDamageComparisonText();
        
        // Atualiza o TextBlock
        SimulateDanoMagicoText->SetText(FText::FromString(ComparisonText));
        
        // Aplica cor baseada na simulação
        float CurrentMagicDamage = ComparisonComponent->GetCurrentMagicDamage();
        float SimulatedMagicDamage = ComparisonComponent->GetSimulatedMagicDamage();
        float Delta = SimulatedMagicDamage - CurrentMagicDamage;
        
        if (FMath::IsNearlyZero(Delta, 0.1f)) // Se valores são iguais
        {
            // Oculta o texto quando não há mudança
            SimulateDanoMagicoText->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            // Mostra o texto com cor apropriada
            if (Delta > 0)
                SimulateDanoMagicoText->SetColorAndOpacity(FLinearColor::Green);
            else
                SimulateDanoMagicoText->SetColorAndOpacity(FLinearColor::Red);
            
            // Torna visível
            SimulateDanoMagicoText->SetVisibility(ESlateVisibility::Visible);
        }
    }
    else
    {
        // Oculta o texto quando não há simulação
        SimulateDanoMagicoText->SetVisibility(ESlateVisibility::Hidden);
    }
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: SimulateDanoMagicoText atualizado"));
}

void UEquipmentStatusWidget::UpdateSimulatedPrecisaoText()
{
    if (!SimulatePrecisaoText)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: SimulatePrecisaoText não encontrado"));
        return;
    }

    if (!ComparisonComponent)
    {
        return;
    }

    // Verifica se há simulação ativa
    if (ComparisonComponent->HasAnySimulation())
    {
        // Pega o texto de comparação do componente
        FString ComparisonText = ComparisonComponent->GetAccuracyComparisonText();
        
        // Atualiza o TextBlock
        SimulatePrecisaoText->SetText(FText::FromString(ComparisonText));
        
        // Aplica cor baseada na simulação
        float CurrentAccuracy = ComparisonComponent->GetCurrentAccuracy();
        float SimulatedAccuracy = ComparisonComponent->GetSimulatedAccuracy();
        float Delta = SimulatedAccuracy - CurrentAccuracy;
        
        if (FMath::IsNearlyZero(Delta, 0.1f)) // Se valores são iguais
        {
            // Oculta o texto quando não há mudança
            SimulatePrecisaoText->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            // Mostra o texto com cor apropriada
            if (Delta > 0)
                SimulatePrecisaoText->SetColorAndOpacity(FLinearColor::Green);
            else
                SimulatePrecisaoText->SetColorAndOpacity(FLinearColor::Red);
            
            // Torna visível
            SimulatePrecisaoText->SetVisibility(ESlateVisibility::Visible);
        }
    }
    else
    {
        // Oculta o texto quando não há simulação
        SimulatePrecisaoText->SetVisibility(ESlateVisibility::Hidden);
    }
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: SimulatePrecisaoText atualizado"));
}

void UEquipmentStatusWidget::UpdateSimulatedResistenciaMagicaText()
{
    if (!SimulateResistenciaMagicaText)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipmentStatus: SimulateResistenciaMagicaText não encontrado"));
        return;
    }

    if (!ComparisonComponent)
    {
        return;
    }

    // Verifica se há simulação ativa
    if (ComparisonComponent->HasAnySimulation())
    {
        // Pega o texto de comparação do componente
        FString ComparisonText = ComparisonComponent->GetMagicResistanceComparisonText();
        
        // Atualiza o TextBlock
        SimulateResistenciaMagicaText->SetText(FText::FromString(ComparisonText));
        
        // Aplica cor baseada na simulação
        float CurrentMagicResistance = ComparisonComponent->GetCurrentMagicResistance();
        float SimulatedMagicResistance = ComparisonComponent->GetSimulatedMagicResistance();
        float Delta = SimulatedMagicResistance - CurrentMagicResistance;
        
        if (FMath::IsNearlyZero(Delta, 0.1f)) // Se valores são iguais
        {
            // Oculta o texto quando não há mudança
            SimulateResistenciaMagicaText->SetVisibility(ESlateVisibility::Hidden);
        }
        else
        {
            // Mostra o texto com cor apropriada
            if (Delta > 0)
                SimulateResistenciaMagicaText->SetColorAndOpacity(FLinearColor::Green);
            else
                SimulateResistenciaMagicaText->SetColorAndOpacity(FLinearColor::Red);
            
            // Torna visível
            SimulateResistenciaMagicaText->SetVisibility(ESlateVisibility::Visible);
        }
    }
    else
    {
        // Oculta o texto quando não há simulação
        SimulateResistenciaMagicaText->SetVisibility(ESlateVisibility::Hidden);
    }
    
    UE_LOG(LogTemp, Log, TEXT("EquipmentStatus: SimulateResistenciaMagicaText atualizado"));
}
