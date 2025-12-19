// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "RPGGameplayAbility.generated.h"

// Forward declaration
class ARPGPartyAIController;

/**
 * Classe base para Gameplay Abilities com suporte a tag de input no projeto RPG
 */
UCLASS()
class RPG_API URPGGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    // Tag usada para ativar a habilidade via input
    UPROPERTY(EditDefaultsOnly, Category="Input")
    FGameplayTag StartupInputTag;

    // Função intermediária que verifica quem está ativando a habilidade
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                const FGameplayEventData* TriggerEventData) override;

    // Função para lógica do player
    virtual void OnPlayerAbilityActivated(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo,
                                          const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData);

    // Função para lógica do AI da party
    virtual void OnPartyAIAbilityActivated(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData);

    // Verifica se o controller é válido e retorna o tipo
    UFUNCTION(BlueprintCallable, Category = "Ability")
    bool IsValidController() const;

    // Retorna true se for PlayerController
    UFUNCTION(BlueprintCallable, Category = "Ability")
    bool IsPlayerController() const;

    // Retorna true se for RPGPartyAIController
    UFUNCTION(BlueprintCallable, Category = "Ability")
    bool IsPartyAIController() const;

    // Retorna o RPGPartyAIController se existir
    UFUNCTION(BlueprintCallable, Category = "Ability")
    ARPGPartyAIController* GetPartyAIController() const;

    // Override de InputPressed/Released para expor como eventos Blueprint
    virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
    virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
    
    // InputHeld customizado (não existe na classe base do Unreal)
    virtual void InputHeld(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);

    // Eventos Blueprint para capturar input durante a habilidade ativa
    UFUNCTION(BlueprintImplementableEvent, Category = "Ability|Input")
    void OnAbilityInputPressed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Ability|Input")
    void OnAbilityInputHeld();

    UFUNCTION(BlueprintImplementableEvent, Category = "Ability|Input")
    void OnAbilityInputReleased();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "RPG|Debug")
	bool bDrawDebug = true;

	// Retorna o TeamId do avatar que possui esta habilidade
	UFUNCTION(BlueprintCallable, Category = "Ability")
	FGenericTeamId GetOwnerTeamId() const;

	// Retorna se deve desenhar debug
	FORCEINLINE bool ShouldDrawDebug() const { return bDrawDebug; }

	// Empurra alvos usando um vetor de velocidade
	void PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& PushVel);

protected:
    // Retorna o cooldown da habilidade (igual ao Aura)
    float GetCooldown(float InLevel = 1.f) const;

    // Retorna o custo total da habilidade (soma todos os modifiers)
    float GetAbilityCost(float InLevel = 1.f) const;
}; 