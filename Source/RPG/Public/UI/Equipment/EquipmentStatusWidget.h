#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "Inventory/Equipment/EquipmentComparisonComponent.h"
#include "EquipmentStatusWidget.generated.h"

class ACharacter;
class UTextBlock;

/**
 * Widget para exibir os 5 atributos core de combate do personagem
 * - Ataque (físico)
 * - Dano Mágico
 * - Precisão
 * - Armadura (defesa física)
 * - Resistência Mágica (defesa mágica)
 */
UCLASS()
class RPG_API UEquipmentStatusWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UEquipmentStatusWidget(const FObjectInitializer& ObjectInitializer);

    // Define o personagem para exibir dados
    UFUNCTION(BlueprintCallable, Category = "Equipment Status")
    void SetTargetCharacter(ACharacter* NewTargetCharacter);

    // Atualiza a exibição dos dados
    UFUNCTION(BlueprintCallable, Category = "Equipment Status")
    void UpdateDisplay();

protected:
    virtual void NativeConstruct() override;

private:
    // Personagem alvo para exibir dados
    UPROPERTY(BlueprintReadOnly, Category = "Equipment Status", meta = (AllowPrivateAccess = "true"))
    ACharacter* TargetCharacter;

    // Widget para exibir o ataque total
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CurrentDamageText = nullptr;

    // Widget para exibir a armadura total
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CurrentArmorText = nullptr;

    // Widget para exibir o dano mágico
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CurrentMagicDamageText = nullptr;

    // Widget para exibir a precisão
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CurrentAccuracyText = nullptr;

    // Widget para exibir a resistência mágica
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CurrentMagicResistanceText = nullptr;

    // === WIDGETS PARA VALORES SIMULADOS ===
    
    // Widget para exibir o ataque simulado
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SimulateAtaqueText = nullptr;

    // Widget para exibir a armadura simulada
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SimulateArmaduraText = nullptr;

    // Widget para exibir o dano mágico simulado
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SimulateDanoMagicoText = nullptr;

    // Widget para exibir a precisão simulada
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SimulatePrecisaoText = nullptr;

    // Widget para exibir a resistência mágica simulada
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SimulateResistenciaMagicaText = nullptr;

    // Pega o ataque total do personagem (incluindo equipamentos)
    float GetTotalDamage();

    // Pega qualquer atributo do personagem (incluindo equipamentos)
    float GetAttributeValue(FGameplayAttribute Attribute);

    // Pega a armadura total do personagem (incluindo equipamentos)
    float GetTotalArmor();

    // Pega o dano mágico total do personagem (incluindo equipamentos)
    float GetTotalMagicDamage();

    // Pega a precisão total do personagem (incluindo equipamentos)
    float GetTotalAccuracy();

    // Pega a resistência mágica total do personagem (incluindo equipamentos)
    float GetTotalMagicResistance();

    // Atualiza o texto do ataque total
    void UpdateTotalDamageText();

    // Atualiza o texto da armadura total
    void UpdateTotalArmorText();

    // Atualiza o texto do dano mágico
    void UpdateTotalMagicDamageText();

    // Atualiza o texto da precisão
    void UpdateTotalAccuracyText();

    // Atualiza o texto da resistência mágica
    void UpdateTotalMagicResistanceText();

    // Atualiza qualquer TextBlock com valor de atributo
    void UpdateAttributeText(UTextBlock* TextWidget, FGameplayAttribute Attribute, const FString& Label);

    // === FUNÇÕES PARA VALORES SIMULADOS ===
    
    // Atualiza todos os valores simulados
    void UpdateSimulatedValues();
    
    // Atualiza texto do ataque simulado
    void UpdateSimulatedAtaqueText();
    
    // Atualiza texto da armadura simulada
    void UpdateSimulatedArmaduraText();
    
    // Atualiza texto do dano mágico simulado
    void UpdateSimulatedDanoMagicoText();
    
    // Atualiza texto da precisão simulada
    void UpdateSimulatedPrecisaoText();
    
    // Atualiza texto da resistência mágica simulada
    void UpdateSimulatedResistenciaMagicaText();

private:
    // Componente de comparação do personagem
    UPROPERTY(BlueprintReadOnly, Category="Equipment Status", meta=(AllowPrivateAccess="true"))
    UEquipmentComparisonComponent* ComparisonComponent = nullptr;
};
