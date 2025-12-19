#include "Inventory/Items/BaseItem.h"
#include "CoreMinimal.h"

#include "UObject/Object.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Character/RPGCharacterBase.h"
#include "Character/RPGCharacter.h"
#include "Inventory/Core/InventorySubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "Inventory/Core/InventorySubsystem.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Engine/Engine.h"

// Construtor
ABaseItem::ABaseItem()
{
    // Configurar este ator para chamar Tick() a cada frame
    PrimaryActorTick.bCanEverTick = true;
    
    // Criar e configurar o componente raiz (colisão)
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetSphereRadius(100.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Interactable"));
    CollisionComponent->SetGenerateOverlapEvents(true);
    SetRootComponent(CollisionComponent);
    
    // Criar o componente de mesh
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Criar o sistema de partículas para destaque visual
    GlowEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GlowEffect"));
    GlowEffect->SetupAttachment(RootComponent);
    GlowEffect->SetAutoActivate(false);
    
    // Configurar replicação para multiplayer
    bReplicates = true;
    SetReplicateMovement(true);
}

void ABaseItem::InitializeFromItemData()
{
    if (!ItemData || !MeshComponent)
    {
        return;
    }

    if (!ItemData->Mesh.IsNull())
    {
        if (UStaticMesh* LoadedMesh = ItemData->Mesh.LoadSynchronous())
        {
            MeshComponent->SetStaticMesh(LoadedMesh);
        }
        else
        {
            MeshComponent->SetStaticMesh(nullptr);
        }
    }
    else
    {
        MeshComponent->SetStaticMesh(nullptr);
    }
}

// Chamado quando o jogo inicia ou quando o item é criado
void ABaseItem::BeginPlay()
{
    Super::BeginPlay();

    // Inicializa a mesh a partir do ItemData
    InitializeFromItemData();

    // Lógica original de bind de overlap
    if (CollisionComponent)
    {
        CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ABaseItem::OnOverlapBegin);
        CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ABaseItem::OnOverlapEnd);
    }
}

// Chamado a cada frame
void ABaseItem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Implementar rotação lenta do item para destaque visual
    if (bShouldRotateWhenInWorld) // Usando a flag que adicionamos
    {
        AddActorLocalRotation(FRotator(0.0f, DeltaTime * 30.0f, 0.0f));
    }
}

// Função para definir o nível do item
void ABaseItem::SetItemLevel(int32 NewLevel)
{
    ItemLevel = FMath::Max(1, NewLevel);
}

// Função para ativar/desativar o efeito de brilho
void ABaseItem::SetGlowEffectActive(bool bActive)
{
    if (GlowEffect)
    {
        if (bActive)
        {
            GlowEffect->Activate();
        }
        else
        {
            GlowEffect->Deactivate();
        }
    }
}

// Chamado quando o item é coletado
void ABaseItem::OnPickedUp(AActor* Collector)
{
    if (!Collector || !ItemData)
    {
        UE_LOG(LogTemp, Warning, TEXT("ABaseItem::OnPickedUp - Collector or ItemData is null."));
        return;
    }

    bool bSuccessfullyAdded = false; // Flag para controlar se a adição foi bem-sucedida
    ARPGCharacterBase* Character = Cast<ARPGCharacterBase>(Collector);

    if (!Character)
    {
         UE_LOG(LogTemp, Warning, TEXT("ABaseItem::OnPickedUp - Collector %s is not an ARPGCharacterBase."), *Collector->GetName());
         // Decidir o que fazer aqui? Por ora, não adicionar e não destruir.
         return;
    }

    if (bAddToInventoryOnPickup)
    {
        // Sempre usar inventário compartilhado
        UInventorySubsystem* Inv = GetGameInstance()->GetSubsystem<UInventorySubsystem>();
        if (Inv)
        {
            EInventoryActionResult Result = Inv->AddItem(ItemData, Quantity, ItemLevel);
            if (Result == EInventoryActionResult::Success)
            {
                bSuccessfullyAdded = true;
            }
        }
    }

    // Notificar o personagem sobre a tentativa de coleta e Destruir o ator APENAS se a adição foi bem-sucedida
    if (bSuccessfullyAdded)
    {
        // Notificação ao personagem removida
        Destroy();
    }
    // Se não foi adicionado com sucesso (inventário cheio, erro, ou bAddToInventoryOnPickup=false), o item permanece no mundo.
}

// Chamado quando um ator entra na área de interação
void ABaseItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Verificar se é um personagem que entrou na área
    ARPGCharacterBase* Character = Cast<ARPGCharacterBase>(OtherActor);
    if (Character && bCanBePickedUp && ItemData)
    {
        // Ativar efeito de brilho
        SetGlowEffectActive(true);
        
        // Mostrar alerta de item
        float Distance = GetDistanceTo(Character);
        
        
        // Chamar o evento Blueprint
        OnPlayerApproach(Character);
    }
}

// Chamado quando um ator sai da área de interação
void ABaseItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Verificar se é um personagem que saiu da área
    ARPGCharacterBase* Character = Cast<ARPGCharacterBase>(OtherActor);
    if (Character)
    {
        // Desativar efeito de brilho
        SetGlowEffectActive(false);
        
        // Chamar o evento Blueprint
        OnPlayerLeave(Character);
    }
}

// --- Implementações da IInteractiveInterface ---

void ABaseItem::Interact_Implementation(ARPGCharacterBase* InteractingCharacter)
{
    UE_LOG(LogTemp, Warning, TEXT("ABaseItem::Interact_Implementation CALLED on %s by %s"), *GetName(), *InteractingCharacter->GetName());

    // Lógica movida da antiga função Interact para cá
    if (!bCanBePickedUp || !ItemData)
    {
        return;
    }
    
    // Chamar o evento Blueprint para implementações específicas
    // Passamos o personagem que interagiu para o evento BP.
    OnInteract(InteractingCharacter); 
    
    // Implementação padrão: coletar o item, SE bPickupOnInteract for true
    if (bPickupOnInteract)
    {
        OnPickedUp(InteractingCharacter); // Passar o personagem que coletou
    }
}

bool ABaseItem::CanInteract_Implementation(ARPGCharacterBase* InteractingCharacter) const
{
    // Um item base pode ser interagido se puder ser pego.
    // Classes filhas podem adicionar mais condições (ex: nível mínimo?).
    return bCanBePickedUp;
}

FText ABaseItem::GetInteractionText_Implementation(ARPGCharacterBase* InteractingCharacter) const
{
    if (ItemData)
    {
        // Exemplo: Retorna "Pegar [Nome do Item]"
        // Você pode querer internacionalizar isso com FText::Format.
        FString ActionText = TEXT("Pegar"); // Ou "Interagir", "Ler", etc. dependendo do item?
        return FText::FromString(FString::Printf(TEXT("%s %s"), *ActionText, *ItemData->ItemName.ToString()));
    }
    return FText::FromString(TEXT("Interagir")); // Texto padrão se não houver ItemData
}

// --- Fim das Implementações da Interface --- 

void ABaseItem::OnDropped()
{
	// Lógica para quando o item é solto
	// Reativar física (se aplicável) e talvez detecção de overlap
	// REMOVER CHAMADA PARA EnableOverlapDetection
	// EnableOverlapDetection(); 
	if(MeshComponent) // Corrigir nome da variável
	{
		MeshComponent->SetSimulatePhysics(true);
	}
}

void ABaseItem::SetItemData(UItemDataAsset* NewData)
{
	ItemData = NewData;
    if (GetWorld() != nullptr)
    {
        InitializeFromItemData();
    }
}

void ABaseItem::SetQuantity(int32 NewQuantity)
{
	// Garante que a quantidade não seja negativa ou zero, a menos que o item possa ter quantidade 0?
	// Por enquanto, vamos manter no mínimo 1.
	Quantity = FMath::Max(1, NewQuantity); 
} 
