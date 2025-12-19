#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "UI/Widget/RPGUserWidget.h"
#include "BaseOverlayWidget.generated.h"

struct FOnAttributeChangeData;

class UProgressBar;
class UTextBlock;
class ARPGCharacter;
class UPartySubsystem;
class UVerticalBox;
class UPartyMemberStatusWidget;

/**
 * Widget base para overlays de personagens
 */
UCLASS(Abstract)
class RPG_API UBaseOverlayWidget : public URPGUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // Método para definir o personagem dono deste widget
    UFUNCTION()
    virtual void SetOwnerCharacter(ARPGCharacter* NewOwner);

    UFUNCTION(BlueprintCallable, Category = "HUD|Character Info")
    void UpdateCharacterName(const FString& NewName);
    


    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat")
    void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);

    void SetWidgetController(UObject* InWidgetController);

protected:
    // Barras principais do personagem ativo
    UPROPERTY(meta = (BindWidget))
    UProgressBar* HealthProgressBar;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* ManaProgressBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HealthText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ManaText;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* XPProgressBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* LevelText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* XPText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* GoldText;

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* PartyMembersContainer;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UPartyMemberStatusWidget> PartyMemberStatusClass;
    


    UFUNCTION()
    virtual void HandleHealthChanged(float Current, float Max);

    UFUNCTION()
    virtual void HandleManaChanged(float Current, float Max);

    // Energy removido - Mana já serve para recursos

    UFUNCTION()
    virtual void HandleXPChanged(ARPGCharacter* Character, int32 CurrentXP, int32 XPForNextLevel);

    UFUNCTION()
    virtual void HandleLevelChanged(ARPGCharacter* Character, int32 NewLevel);

    UFUNCTION()
    virtual void HandleGoldChanged(int32 NewAmount);

    UFUNCTION()
    virtual void OnCharacterAttributesReady();
    

    UFUNCTION(BlueprintCallable, Category = "Widget Controller")
    void BindToController();

    UFUNCTION()
    void OnHealthChangedFromController(float NewValue);
    
    UFUNCTION()
    void OnMaxHealthChangedFromController(float NewValue);
    
    UFUNCTION()
    void OnManaChangedFromController(float NewValue);
    
    UFUNCTION()
    void OnMaxManaChangedFromController(float NewValue);
    
    // Energy removido - Mana já serve para recursos

    void OnHealthChangedDirect(const FOnAttributeChangeData& Data);
    void OnMaxHealthChangedDirect(const FOnAttributeChangeData& Data);
    void OnManaChangedDirect(const FOnAttributeChangeData& Data);
    void OnMaxManaChangedDirect(const FOnAttributeChangeData& Data);
    // Energy removido - Mana já serve para recursos
    
    // Função para atualizar valores iniciais
    void UpdateInitialValues();
    
    // === SKILL ICONS ===
    
    /** Atualiza todos os ícones de habilidades */
    UFUNCTION(BlueprintCallable, Category = "Skill Icons")
    void UpdateAllSkillIcons();
    
    /** Conecta aos delegates do SkillEquipmentComponent */
    UFUNCTION(BlueprintCallable, Category = "Skill Icons")
    void ConnectToSkillEquipmentEvents();
    
    /** Desconecta dos delegates do SkillEquipmentComponent */
    UFUNCTION(BlueprintCallable, Category = "Skill Icons")
    void DisconnectFromSkillEquipmentEvents();

protected:
    virtual void BeginDestroy() override;

    // === DELEGATE CALLBACKS ===
    
    /** Chamado quando uma habilidade é equipada */
    UFUNCTION()
    void OnSkillEquipped(FName CharacterID, FName SlotID, FName SkillID);
    
    /** Chamado quando uma habilidade é desequipada */
    UFUNCTION()
    void OnSkillUnequipped(FName CharacterID, FName SlotID, FName SkillID);
    
    /** Chamado quando uma habilidade é equipada em um slot específico */
    UFUNCTION()
    void OnSkillEquippedInSlot(FName SlotID, FName SkillID, UTexture2D* SkillIcon);
    
    /** Conecta um array de SkillIconWidgets */
    UFUNCTION(BlueprintCallable, Category = "Skill Icons")
    void ConnectSkillIconWidgetsArray(const TArray<USkillIconWidget*>& IconWidgetsArray);
    
    /** Adiciona um SkillIconWidget ao gerenciamento */
    UFUNCTION(BlueprintCallable, Category = "Skill Icons")
    void AddSkillIconWidget(USkillIconWidget* IconWidget);
    
    /** Remove um SkillIconWidget do gerenciamento */
    UFUNCTION(BlueprintCallable, Category = "Skill Icons")
    void RemoveSkillIconWidget(USkillIconWidget* IconWidget);

    // Referências importantes
    UPROPERTY()
    ARPGCharacter* ActiveCharacter;

    UPROPERTY()
    UPartySubsystem* PartySubsystem;

    // Certifique-se que esta variável está declarada e vinculada no editor (se for UMG)
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* CharacterNameText;



private:
    // Array para pool de widgets de status dos membros
    UPROPERTY()
    TArray<UPartyMemberStatusWidget*> PartyMemberWidgetPool;

    // Timer handle para atualização forçada (verificar se ainda é usado)
    FTimerHandle UpdateTimerHandle;

    // Valores atuais dos atributos (para sistema MVC)
    float CurrentHealthValue = 0.0f;
    float MaxHealthValue = 0.0f;
    float CurrentManaValue = 0.0f;
    float MaxManaValue = 0.0f;
    // Energy removido - Mana já serve para recursos
    
    // === SKILL ICON WIDGETS ===
    
    /** Lista de SkillIconWidgets gerenciados */
    UPROPERTY()
    TArray<TObjectPtr<class USkillIconWidget>> ManagedSkillIconWidgets;

    void BindDelegates();
    void UnbindDelegates();
    void SetupInitialValues();
    void UpdateAttributeText(UTextBlock* TextWidget, float Current, float Max);
    void UpdateActiveCharacterBars();

    // Renomeado: Atualiza a exibição dos widgets da party (reutilizando)
    void UpdatePartyMemberDisplay();
    // Renomeado: Esconde e reseta os widgets no pool
    void HideAndResetPartyMemberWidgets();

    // Novas funções para atualização forçada (verificar se ainda são usadas)
    void ForceUpdateAllValues();
    void UpdateAttributeValue(UProgressBar* Bar, UTextBlock* Text, float Current, float Max);
}; 