// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "RPGAbilitySystemComponent.generated.h"

/**
 * Componente customizado de Ability System para o RPG
 */
UCLASS()
class RPG_API URPGAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()
public:
    // Concede as habilidades iniciais ao personagem
    void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
    // Concede as habilidades passivas iniciais ao personagem
    void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);
    // Dispara callbacks de entrada para habilidades baseadas em tag dinâmica
   
    /** Chama callback para tag de input pressionada */
    void AbilityInputTagHeld(const FGameplayTag& InputTag);
    /** Chama callback para tag de input pressionada (alias) */
    void AbilityInputTagPressed(const FGameplayTag& InputTag);
    void AbilityInputTagReleased(const FGameplayTag& InputTag);
    /** Notifica que o AbilityActorInfo foi inicializado */
    void AbilityActorInfoSet();

    /** Executa o upgrade de um atributo via tag */
    UFUNCTION(BlueprintCallable, Category="AbilitySystem")
    void UpgradeAttribute(const FGameplayTag& AttributeTag);
    /** RPC para servidor processar upgrade de atributo */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);

    /** Retorna o Avatar Actor (owner do ASC) */
    UFUNCTION(BlueprintCallable, Category="AbilitySystem")
    AActor* GetAvatarActor() const;

    // Retorna true se existir uma habilidade ATIVA cujo spec contenha a InputTag informada
    UFUNCTION(BlueprintPure, Category="AbilitySystem|Input")
    bool HasActiveAbilityWithInputTag(const FGameplayTag& InputTag) const;

    // === ABILITY LEVEL MANAGEMENT ===

    /** Define o nível da ability diretamente (substitui o nível atual) */
    UFUNCTION(BlueprintCallable, Category = "RPG|Abilities")
    void SetAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level);

    /** Adiciona nível à ability gradualmente (incrementa o nível atual) */
    UFUNCTION(BlueprintCallable, Category = "RPG|Abilities")
    void AddToAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level = 1);

private:
    /** RPC para servidor processar SetAbilityLevel */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level);
    bool ServerSetAbilityLevel_Validate(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level) { return true; }
    void ServerSetAbilityLevel_Implementation(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level);

    /** RPC para servidor processar AddToAbilityLevel */
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerAddToAbilityLevel(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level);
    bool ServerAddToAbilityLevel_Validate(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level) { return true; }
    void ServerAddToAbilityLevel_Implementation(TSubclassOf<UGameplayAbility> AbilityClass, int32 Level);

    // === OVERRIDES ===

    virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
    virtual void OnRep_ActivateAbilities() override;

private:
    void HandleAutoActivateAbility(const FGameplayAbilitySpec& AbilitySpec);

}; 