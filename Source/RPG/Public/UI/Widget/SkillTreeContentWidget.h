// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "SkillTreeContentWidget.generated.h"



class UTextBlock;
class UVerticalBox;
class UScrollBox;
class USkillNodeWidget;
class USkillTreeWidgetController;
class ARPGCharacter;
class UComboBoxString;
class UPanelWidget;

// Delegate para mudança de personagem
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterChanged, FName, CharacterID);

/**
 * Widget de conteúdo para a aba Skill Tree no menu de jogo
 * Permite visualizar e desbloquear habilidades do personagem
 */
UCLASS()
class RPG_API USkillTreeContentWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    /** Define o controller para este widget */
    UFUNCTION(BlueprintCallable, Category = "Widget Controller")
    void SetWidgetController(USkillTreeWidgetController* InWidgetController);

    /** Atualiza todos os nós da árvore */
    void UpdateAllNodes();
    
    /** Busca um personagem pelo CharacterID */
    UFUNCTION(BlueprintCallable, Category = "Skill Tree")
    ARPGCharacter* GetCharacterByID(const FName& CharacterID) const;
    
    /** Retorna o personagem atual */
    UFUNCTION(BlueprintPure, Category = "Skill Tree")
    ARPGCharacter* GetCurrentCharacter() const { return CurrentCharacter; }
    
    // === PAN ===
    
    /** Resetar posição do PAN */
    UFUNCTION(BlueprintCallable, Category = "SkillTree|PAN")
    void ResetPanPosition();
    
    /** Definir se o PAN está ativo */
    UFUNCTION(BlueprintCallable, Category = "SkillTree|PAN")
    void SetPanEnabled(bool bEnable);
    
    /** Obter posição atual do PAN */
    UFUNCTION(BlueprintPure, Category = "SkillTree|PAN")
    FVector2D GetCurrentPanOffset() const { return CurrentPanOffset; }
    
    // === ZOOM ===
    
    /** Resetar zoom para padrão */
    UFUNCTION(BlueprintCallable, Category = "SkillTree|ZOOM")
    void ResetZoom();
    
    /** Definir se o ZOOM está ativo */
    UFUNCTION(BlueprintCallable, Category = "SkillTree|ZOOM")
    void SetZoomEnabled(bool bEnable);
    
    /** Obter zoom atual */
    UFUNCTION(BlueprintPure, Category = "SkillTree|ZOOM")
    float GetCurrentZoom() const { return CurrentZoom; }
    
    /** Delegate que ENVIA CharacterID quando personagem muda */
    UPROPERTY(BlueprintAssignable, Category = "Skill Tree")
    FOnCharacterChanged OnCharacterChanged;
    

    
protected:
    // === WIDGETS DO HEADER ===
    
    /** ComboBox para selecionar o personagem */
    UPROPERTY(meta = (BindWidget))
    UComboBoxString* CharacterSelector;
    
    /** Nome do personagem */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CharacterNameText;
    
    /** Nível do personagem */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CharacterLevelText;
    
    /** Pontos de magia disponíveis */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SpellPointsText;
    
    // === CONTAINER DA ÁRVORE ===
    
    /** Container principal da árvore de habilidades */
    UPROPERTY(meta = (BindWidget))
    class UCanvasPanel* TreeContainer;
    
    // === FUNÇÕES ===
    
    /** Inicializa a árvore de habilidades */
    void InitializeSkillTree();
    
    /** Inicializa o ComboBox com os personagens do grupo */
    void InitializeCharacterSelector();
    
    /** Atualiza o header com informações do personagem */
    void UpdateHeader();
    
    // === PAN ===
    
    /** Configura o sistema de PAN */
    void SetupPan();
    
    /** Aplica o PAN ao container */
    void ApplyPanToContainer();
    
    /** Aplica limites ao offset do PAN */
    void ClampPanOffset(FVector2D& Offset);
    
    // === ZOOM ===
    
    /** Aplica o ZOOM ao container */
    void ApplyZoomToContainer();
    
    // === HANDLERS DE EVENTOS ===
    
    /** Handler para quando o personagem selecionado muda */
    UFUNCTION()
    void OnCharacterSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
    
    /** Handler para quando os pontos de magia mudam */
    UFUNCTION()
    void OnPointsChanged();
    
    // === EVENTOS DO PAN ===
    
    /** Mouse down para iniciar PAN */
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    /** Mouse move para PAN */
    virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    /** Mouse up para parar PAN */
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    // === EVENTOS DO ZOOM ===
    
    /** Mouse wheel para ZOOM */
    virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    // === CONFIGURAÇÃO DO PAN ===
    
    /** Se o PAN está ativo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|PAN")
    bool bPanEnabled = true;
    
    /** Velocidade do PAN */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|PAN")
    float PanSpeed = 1.0f;
    
    /** Limites mínimos do PAN */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|PAN")
    FVector2D MinPanOffset = FVector2D(-1000, -750);
    
    /** Limites máximos do PAN */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|PAN")
    FVector2D MaxPanOffset = FVector2D(1000, 750);
    
    // === CONFIGURAÇÃO DO ZOOM ===
    
    /** Se o ZOOM está ativo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|ZOOM")
    bool bZoomEnabled = true;
    
    /** Velocidade do ZOOM */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|ZOOM")
    float ZoomSpeed = 0.1f;
    
    /** Zoom mínimo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|ZOOM")
    float MinZoom = 0.5f;
    
    /** Zoom máximo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|ZOOM")
    float MaxZoom = 3.0f;
    
    /** Zoom padrão */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|ZOOM")
    float DefaultZoom = 1.0f;
    
    // === CONFIGURAÇÃO DO ZOOM SUAVE ===
    
    /** Se o ZOOM SUAVE está ativo */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|ZOOM|Smooth")
    bool bSmoothZoomEnabled = true;
    
    /** Velocidade da suavização do ZOOM */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillTree|ZOOM|Smooth")
    float SmoothZoomSpeed = 8.0f;
    
private:
    // === DADOS INTERNOS ===
    
    /** Referência ao personagem atual */
    UPROPERTY()
    ARPGCharacter* CurrentCharacter;
    
    /** Mapa de nomes no ComboBox para referências de personagens */
    UPROPERTY()
    TMap<FString, ARPGCharacter*> CharacterMap;
    
    /** Widget Controller */
    UPROPERTY()
    USkillTreeWidgetController* WidgetController;
    
    // === ESTADO DO PAN ===
    
    /** Se está fazendo PAN */
    bool bIsPanning = false;
    
    /** Posição inicial do mouse */
    FVector2D LastMousePosition;
    
    /** Offset atual do PAN */
    FVector2D CurrentPanOffset = FVector2D::ZeroVector;
    
    // === ESTADO DO ZOOM ===
    
    /** Zoom atual */
    float CurrentZoom = 1.0f;
    
    /** Zoom alvo para suavização */
    float TargetZoom = 1.0f;
    
    /** Se está fazendo zoom suave */
    bool bIsSmoothZooming = false;
}; 