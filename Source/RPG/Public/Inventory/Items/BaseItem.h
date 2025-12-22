#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/Core/InventoryEnums.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Interfaces/InteractiveInterface.h"
#include "BaseItem.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class ARPGCharacterBase;

/**
 * Classe base para todos os itens coletáveis do mundo
 */
UCLASS(BlueprintType, Blueprintable)
class RPG_API ABaseItem : public AActor, public IInteractiveInterface
{
    GENERATED_BODY()
    
public:    
    // Construtor padrão
    ABaseItem();

protected:
    // Chamado quando o jogo inicia ou quando o item é criado
    virtual void BeginPlay() override;

    // Inicializa a aparência do item com base no ItemData
    void InitializeFromItemData(); 

    // Componente de mesh para representação visual
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;
    
    // Componente de colisão para interação
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionComponent;
    
    // Efeito Niagara para destaque
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UNiagaraComponent* GlowEffect;

    // Asset do sistema Niagara para o efeito de brilho (editável no Blueprint)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
    TObjectPtr<UNiagaraSystem> GlowEffectAsset;

    // Se verdadeiro, usa delay antes de destruir o actor quando coletado
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Destroy")
    bool bUseDestroyDelay = false;

    // Delay em segundos antes de destruir o actor (usado se bUseDestroyDelay = true)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Destroy", meta = (EditCondition = "bUseDestroyDelay", ClampMin = "0.0"))
    float DestroyDelay = 1.0f;
    
    // Dados do item (definidos em blueprints/instâncias)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    TObjectPtr<UItemDataAsset> ItemData;
    
    // Quantidade do item
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 Quantity = 1;
    
    // Nível do item
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
    int32 ItemLevel = 1;
    
    /** Se verdadeiro, o item irá girar lentamente sobre seu eixo Z no Tick. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Visual")
    bool bShouldRotateWhenInWorld = true; // Valor padrão é true
    
    // Se o item pode ser coletado atualmente
    UPROPERTY(BlueprintReadWrite, Category = "Item")
    bool bCanBePickedUp = true;
    
    /** Se verdadeiro, chamar Interact() também chamará OnPickedUp() por padrão. Desmarque para itens que só usam OnInteract. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Interaction")
    bool bPickupOnInteract = true;
    
    /** Se verdadeiro, chamar OnPickedUp tentará adicionar este item ao inventário do coletor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Interaction")
    bool bAddToInventoryOnPickup = true;
    
    // Callback para quando um ator entra na área de interação
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    
    // Callback para quando um ator sai da área de interação
    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:    
    //~ Begin IInteractiveInterface
    virtual void Interact_Implementation(ARPGCharacterBase* InteractingCharacter) override;
    virtual bool CanInteract_Implementation(ARPGCharacterBase* InteractingCharacter) const override;
    virtual FText GetInteractionText_Implementation(ARPGCharacterBase* InteractingCharacter) const override;
    //~ End IInteractiveInterface

    //~ Begin IGenericTeamAgentInterface
    // ... (código da interface de time)
    //~ End IGenericTeamAgentInterface

    // Chamado a cada frame
    virtual void Tick(float DeltaTime) override;
    
    // Evento Blueprint chamado quando o jogador interage com o item
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void OnInteract(AActor* Interactor);
    
    // Evento Blueprint chamado quando o jogador entra na área de interação
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void OnPlayerApproach(class ARPGCharacterBase* Player);
    
    // Evento Blueprint chamado quando o jogador sai da área de interação
    UFUNCTION(BlueprintImplementableEvent, Category = "Item")
    void OnPlayerLeave(class ARPGCharacterBase* Player);
    
    // Retorna os dados do item
    UFUNCTION(BlueprintPure, Category = "Item")
    UItemDataAsset* GetItemData() const { return ItemData; }
    
    // Retorna a quantidade do item
    UFUNCTION(BlueprintPure, Category = "Item")
    int32 GetQuantity() const { return Quantity; }
    
    // Retorna o nível do item
    UFUNCTION(BlueprintPure, Category = "Item")
    int32 GetItemLevel() const { return ItemLevel; }
    
    // Verifica se o item pode ser coletado
    UFUNCTION(BlueprintPure, Category = "Item")
    bool CanBePickedUp() const { return bCanBePickedUp; }
    
    // Define o nível do item
    UFUNCTION(BlueprintCallable, Category = "Item")
    void SetItemLevel(int32 NewLevel);
    
    // Ativa ou desativa o efeito de brilho
    UFUNCTION(BlueprintCallable, Category = "Item|Visual")
    void SetGlowEffectActive(bool bActive);
    
    // Chamado quando o item é coletado
    UFUNCTION(BlueprintCallable, Category = "Item")
    virtual void OnPickedUp(AActor* Collector);

    virtual void OnDropped();

private:
    // Função chamada após o delay para destruir o actor
    void DelayedDestroy();

    // --- Setters ---
    UFUNCTION(BlueprintCallable, Category = "Item") // Expor ao Blueprint opcionalmente
    void SetItemData(UItemDataAsset* NewData);

    // Define a quantidade do item (manter esta)
    UFUNCTION(BlueprintCallable, Category = "Item")
    void SetQuantity(int32 NewQuantity);
}; 
