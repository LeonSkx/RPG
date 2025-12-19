#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Inventory/Core/InventoryTypes.h"
#include "Inventory/Equipment/EquipmentComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayAbilitySpec.h"
#include "EquippedItem.generated.h"

class UItemDataAsset;

/**
 * Modificador de stat aplicado ao personagem quando item está equipado
 */
USTRUCT(BlueprintType)
struct FAppliedStatModifier
{
    GENERATED_BODY()

    // Tipo de atributo afetado
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    EAttributeType AttributeType = EAttributeType::None;

    // Valor aplicado
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    float AppliedValue = 0.0f;

    // Se é percentual
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    bool bIsPercentage = false;

    FAppliedStatModifier() = default;
    
    FAppliedStatModifier(EAttributeType InType, float InValue, bool InIsPercentage)
        : AttributeType(InType), AppliedValue(InValue), bIsPercentage(InIsPercentage) {}
};

/**
 * Representa um item equipado com dados dinâmicos e representação visual
 */
UCLASS(BlueprintType)
class RPG_API UEquippedItem : public UObject
{
    GENERATED_BODY()

public:
    UEquippedItem();

    /**
     * Inicializa o item equipado baseado em um FInventoryItem
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void InitializeFromInventoryItem(const FInventoryItem& InventoryItem, EEquipmentSlot InSlot);

    /**
     * Anexa o item visualmente ao socket do personagem
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    bool AttachToSocket(AActor* OwnerActor, const FString& SocketName);

    /**
     * Remove o item visualmente do personagem
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void DetachFromSocket();

    /**
     * Aplica os stats do item ao personagem (será implementado quando tivermos sistema de atributos)
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void ApplyStatsToCharacter(AActor* Character);

    /**
     * Remove os stats do item do personagem
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void RemoveStatsFromCharacter(AActor* Character);

    /**
     * Converte de volta para FInventoryItem (para desequipar)
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    FInventoryItem ConvertBackToInventoryItem() const;

    /**
     * Atualiza o nível do item equipado e reaplica os efeitos
     */
    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void UpgradeItem(int32 NewLevel);

    // --- Getters ---
    
    UFUNCTION(BlueprintPure, Category = "Equipment")
    const FInventoryItem& GetSourceItem() const { return SourceItem; }

    UFUNCTION(BlueprintPure, Category = "Equipment")
    UItemDataAsset* GetItemData() const { return SourceItem.ItemData; }

    UFUNCTION(BlueprintPure, Category = "Equipment")
    EEquipmentSlot GetEquipmentSlot() const { return EquipmentSlot; }

    UFUNCTION(BlueprintPure, Category = "Equipment")
    bool IsVisuallyAttached() const { return AttachedMeshComponent != nullptr; }

    UFUNCTION(BlueprintPure, Category = "Equipment")
    const TArray<FAppliedStatModifier>& GetAppliedStats() const { return AppliedStats; }

protected:
    // Item original do inventário
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    FInventoryItem SourceItem;

    // Slot onde está equipado
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    EEquipmentSlot EquipmentSlot = EEquipmentSlot::Weapon;

    // Componente visual anexado ao personagem
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    TObjectPtr<UStaticMeshComponent> AttachedMeshComponent;

    // Socket onde está anexado atualmente
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    FString CurrentAttachSocket;

    // Stats atualmente aplicados ao personagem
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    TArray<FAppliedStatModifier> AppliedStats;

    // Handles dos GameplayEffects aplicados ao equipar (se houverem)
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    TArray<FActiveGameplayEffectHandle> EquippedEffectHandles;

    // Handles das GameplayAbilities concedidas ao equipar (se houverem)
    UPROPERTY(BlueprintReadOnly, Category = "Equipment")
    TArray<FGameplayAbilitySpecHandle> EquippedAbilityHandles;

private:
    /**
     * Carrega a mesh do item (sync ou async)
     */
    UStaticMesh* LoadItemMesh() const;

    /**
     * Cria o componente visual para o item
     */
    UStaticMeshComponent* CreateMeshComponent(AActor* OwnerActor);
}; 