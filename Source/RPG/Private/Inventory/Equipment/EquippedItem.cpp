#include "Inventory/Equipment/EquippedItem.h"
#include "Inventory/Items/ItemDataAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"

UEquippedItem::UEquippedItem()
{
    AttachedMeshComponent = nullptr;
}

void UEquippedItem::InitializeFromInventoryItem(const FInventoryItem& InventoryItem, EEquipmentSlot InSlot)
{
    if (!InventoryItem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::InitializeFromInventoryItem - Item inválido!"));
        return;
    }

    SourceItem = InventoryItem;
    EquipmentSlot = InSlot;

    UE_LOG(LogTemp, Log, TEXT("UEquippedItem inicializado: %s (Level %d) no slot %s"), 
        *SourceItem.ItemData->ItemName.ToString(),
        SourceItem.Level,
        *UEnum::GetValueAsString(EquipmentSlot));
}

bool UEquippedItem::AttachToSocket(AActor* OwnerActor, const FString& SocketName)
{
    if (!OwnerActor || SocketName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::AttachToSocket - OwnerActor ou SocketName inválido!"));
        return false;
    }

    if (!SourceItem.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::AttachToSocket - Item não inicializado!"));
        return false;
    }

    // Se já está anexado, desanexar primeiro
    if (AttachedMeshComponent)
    {
        DetachFromSocket();
    }

    // Encontrar o SkeletalMeshComponent do personagem
    USkeletalMeshComponent* SkeletalMesh = OwnerActor->FindComponentByClass<USkeletalMeshComponent>();
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::AttachToSocket - SkeletalMeshComponent não encontrado em %s"), 
            *OwnerActor->GetName());
        return false;
    }

    // Verificar se o socket existe
    if (!SkeletalMesh->DoesSocketExist(FName(*SocketName)))
    {
        UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::AttachToSocket - Socket '%s' não existe em %s"), 
            *SocketName, *OwnerActor->GetName());
        return false;
    }

    // Criar componente visual
    AttachedMeshComponent = CreateMeshComponent(OwnerActor);
    if (!AttachedMeshComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("UEquippedItem::AttachToSocket - Falhou ao criar componente visual!"));
        return false;
    }

    // Anexar ao socket
    AttachedMeshComponent->AttachToComponent(
        SkeletalMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        FName(*SocketName)
    );

    CurrentAttachSocket = SocketName;

    UE_LOG(LogTemp, Log, TEXT("UEquippedItem::AttachToSocket - %s anexado ao socket '%s' com sucesso!"), 
        *SourceItem.ItemData->ItemName.ToString(), *SocketName);

    return true;
}

void UEquippedItem::DetachFromSocket()
{
    if (AttachedMeshComponent)
    {
        UE_LOG(LogTemp, Log, TEXT("UEquippedItem::DetachFromSocket - Removendo %s do socket '%s'"), 
            *SourceItem.ItemData->ItemName.ToString(), *CurrentAttachSocket);

        AttachedMeshComponent->DestroyComponent();
        AttachedMeshComponent = nullptr;
        CurrentAttachSocket.Empty();
    }
}

void UEquippedItem::ApplyStatsToCharacter(AActor* Character)
{
    // TODO: Implementar quando tivermos sistema de atributos
    // Por enquanto, apenas log dos stats que seriam aplicados
    
    if (AppliedStats.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("UEquippedItem::ApplyStatsToCharacter - Aplicando %d modificadores de %s:"), 
            AppliedStats.Num(), *SourceItem.ItemData->ItemName.ToString());

        for (const FAppliedStatModifier& StatMod : AppliedStats)
        {
            UE_LOG(LogTemp, Log, TEXT("  - %s: %s%.1f"), 
                *UEnum::GetValueAsString(StatMod.AttributeType),
                StatMod.bIsPercentage ? TEXT("%") : TEXT("+"),
                StatMod.AppliedValue);
        }
    }

    // Aplicar efeitos GAS do equipamento
    if (Character && SourceItem.ItemData && SourceItem.ItemData->EquipmentGameplayEffects.Num() > 0)
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
        {
            UE_LOG(LogTemp, Log, TEXT("UEquippedItem::ApplyStatsToCharacter - Aplicando %d efeitos GAS para %s"), 
                SourceItem.ItemData->EquipmentGameplayEffects.Num(), *SourceItem.ItemData->ItemName.ToString());

            // Limpar handles antigos
            EquippedEffectHandles.Empty();

            // Aplicar cada efeito GAS
            for (const FEquipmentGameplayEffect& EffectData : SourceItem.ItemData->EquipmentGameplayEffects)
            {

                if (EffectData.GameplayEffectClass)
                {
                    // Criar contexto do efeito
                    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
                    EffectContext.AddSourceObject(Character);

                    // Criar spec do efeito
                    FGameplayEffectSpecHandle EffectSpec = ASC->MakeOutgoingSpec(
                        EffectData.GameplayEffectClass, 
                        EffectData.EffectLevel, 
                        EffectContext
                    );

                    if (EffectSpec.IsValid())
                    {
                        // Aplicar o efeito
                        FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
                        
                        if (ActiveHandle.IsValid())
                        {
                            EquippedEffectHandles.Add(ActiveHandle);
                            UE_LOG(LogTemp, Log, TEXT("  - Efeito GAS aplicado: %s (Level %d)"), 
                                *EffectData.GameplayEffectClass->GetName(), EffectData.EffectLevel);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("  - Falhou ao aplicar efeito GAS: %s"), 
                                *EffectData.GameplayEffectClass->GetName());
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("  - Spec inválido para efeito GAS: %s"), 
                            *EffectData.GameplayEffectClass->GetName());
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("  - GameplayEffectClass nulo"));
                }
            }

            UE_LOG(LogTemp, Log, TEXT("UEquippedItem::ApplyStatsToCharacter - %d efeitos GAS aplicados com sucesso"), 
                EquippedEffectHandles.Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::ApplyStatsToCharacter - ASC não encontrado em %s"), 
                *Character->GetName());
        }
    }

    // Conceder abilities GAS do equipamento
    if (Character && SourceItem.ItemData && SourceItem.ItemData->EquipmentGameplayAbilities.Num() > 0)
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
        {
            UE_LOG(LogTemp, Log, TEXT("UEquippedItem::ApplyStatsToCharacter - Concedendo %d abilities GAS para %s"), 
                SourceItem.ItemData->EquipmentGameplayAbilities.Num(), *SourceItem.ItemData->ItemName.ToString());

            // Limpar handles antigos
            EquippedAbilityHandles.Empty();

            // Conceder cada ability GAS
            for (const FEquipmentGameplayAbility& AbilityData : SourceItem.ItemData->EquipmentGameplayAbilities)
            {
                if (AbilityData.GameplayAbilityClass)
                {
                    // Verificar se é uma RPGGameplayAbility válida
                    if (const URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(AbilityData.GameplayAbilityClass->GetDefaultObject()))
                    {
                        // Criar spec da ability
                        FGameplayAbilitySpec AbilitySpec(AbilityData.GameplayAbilityClass, AbilityData.AbilityLevel);
                        
                        // Configurar tag de input (usando RPGGameplayAbility se disponível)
                        if (AbilityData.InputTag.IsValid())
                        {
                            AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityData.InputTag);
                            UE_LOG(LogTemp, Log, TEXT("    - Configurada para input: %s"), *AbilityData.InputTag.ToString());
                        }
                        else if (RPGAbility->StartupInputTag.IsValid())
                        {
                            // Usar a tag padrão da ability se não foi especificada
                            AbilitySpec.GetDynamicSpecSourceTags().AddTag(RPGAbility->StartupInputTag);
                            UE_LOG(LogTemp, Log, TEXT("    - Usando input padrão da ability: %s"), *RPGAbility->StartupInputTag.ToString());
                        }

                        // Conceder a ability
                        FGameplayAbilitySpecHandle AbilityHandle = ASC->GiveAbility(AbilitySpec);
                        
                        if (AbilityHandle.IsValid())
                        {
                            EquippedAbilityHandles.Add(AbilityHandle);
                            UE_LOG(LogTemp, Log, TEXT("  - RPGGameplayAbility concedida: %s (Level %d)"), 
                                *AbilityData.GameplayAbilityClass->GetName(), AbilityData.AbilityLevel);

                            // Ativar automaticamente se configurado para ativar ao equipar
                            if (AbilityData.bActivateOnEquip)
                            {
                                ASC->TryActivateAbility(AbilityHandle);
                                UE_LOG(LogTemp, Log, TEXT("    - Ability ativada automaticamente (passiva)"));
                            }
                            else
                            {
                                UE_LOG(LogTemp, Log, TEXT("    - Ability configurada para ativação manual"));
                            }
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("  - Falhou ao conceder RPGGameplayAbility: %s"), 
                                *AbilityData.GameplayAbilityClass->GetName());
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("  - Ability não é uma RPGGameplayAbility válida: %s"), 
                            *AbilityData.GameplayAbilityClass->GetName());
                    }
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("  - GameplayAbilityClass nulo"));
                }
            }

            UE_LOG(LogTemp, Log, TEXT("UEquippedItem::ApplyStatsToCharacter - %d abilities GAS concedidas com sucesso"), 
                EquippedAbilityHandles.Num());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::ApplyStatsToCharacter - ASC não encontrado em %s"), 
                *Character->GetName());
        }
    }
}

void UEquippedItem::RemoveStatsFromCharacter(AActor* Character)
{
    // TODO: Implementar quando tivermos sistema de atributos
    // Por enquanto, apenas log
    
    if (AppliedStats.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("UEquippedItem::RemoveStatsFromCharacter - Removendo %d modificadores de %s"), 
            AppliedStats.Num(), *SourceItem.ItemData->ItemName.ToString());
    }

    // Remover GEs aplicados, se existirem
    if (Character && EquippedEffectHandles.Num() > 0)
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
        {
            for (FActiveGameplayEffectHandle& Handle : EquippedEffectHandles)
            {
                if (Handle.IsValid())
                {
                    ASC->RemoveActiveGameplayEffect(Handle);
                }
            }
            UE_LOG(LogTemp, Log, TEXT("UEquippedItem::RemoveStatsFromCharacter - %d GE(s) removidos"), EquippedEffectHandles.Num());
        }
        EquippedEffectHandles.Empty();
    }

    // Remover abilities GAS concedidas, se existirem
    if (Character && EquippedAbilityHandles.Num() > 0)
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
        {
            for (FGameplayAbilitySpecHandle& Handle : EquippedAbilityHandles)
            {
                if (Handle.IsValid())
                {
                    ASC->ClearAbility(Handle);
                }
            }
            UE_LOG(LogTemp, Log, TEXT("UEquippedItem::RemoveStatsFromCharacter - %d ability(s) removidas"), EquippedAbilityHandles.Num());
        }
        EquippedAbilityHandles.Empty();
    }
}

FInventoryItem UEquippedItem::ConvertBackToInventoryItem() const
{
    return SourceItem;
}

void UEquippedItem::UpgradeItem(int32 NewLevel)
{
    if (!SourceItem.IsValid() || SourceItem.Level == NewLevel)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("UEquippedItem::UpgradeItem - %s: Level %d → %d"), 
        *SourceItem.ItemData->ItemName.ToString(), SourceItem.Level, NewLevel);

    // Encontrar o personagem dono
    AActor* OwnerActor = nullptr;
    if (AttachedMeshComponent)
    {
        OwnerActor = AttachedMeshComponent->GetOwner();
    }

    if (OwnerActor)
    {
        // 1. Remover efeitos antigos
        RemoveStatsFromCharacter(OwnerActor);
        
        // 2. Atualizar nível do item
        SourceItem.Level = NewLevel;
        
        // 3. Reaplicar efeitos com novo nível
        ApplyStatsToCharacter(OwnerActor);
        
        UE_LOG(LogTemp, Log, TEXT("UEquippedItem::UpgradeItem - %s atualizado para Level %d com sucesso!"), 
            *SourceItem.ItemData->ItemName.ToString(), NewLevel);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::UpgradeItem - OwnerActor não encontrado para %s"), 
            *SourceItem.ItemData->ItemName.ToString());
    }
}

UStaticMesh* UEquippedItem::LoadItemMesh() const
{
    if (!SourceItem.ItemData)
    {
        return nullptr;
    }

    UStaticMesh* Mesh = nullptr;

    // Tentar carregar da soft reference
    if (!SourceItem.ItemData->Mesh.IsNull())
    {
        Mesh = SourceItem.ItemData->Mesh.LoadSynchronous();
    }

    // Fallback: tentar carregar do path direto
    if (!Mesh && !SourceItem.ItemData->ItemMeshPath.IsEmpty())
    {
        Mesh = LoadObject<UStaticMesh>(nullptr, *SourceItem.ItemData->ItemMeshPath);
    }

    if (!Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::LoadItemMesh - Falhou ao carregar mesh para %s"), 
            *SourceItem.ItemData->ItemName.ToString());
    }

    return Mesh;
}



UStaticMeshComponent* UEquippedItem::CreateMeshComponent(AActor* OwnerActor)
{
    UStaticMesh* Mesh = LoadItemMesh();
    if (!Mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("UEquippedItem::CreateMeshComponent - Mesh não carregada para %s"), 
            *SourceItem.ItemData->ItemName.ToString());
        return nullptr;
    }

    // Criar componente
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(OwnerActor);
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("UEquippedItem::CreateMeshComponent - Falhou ao criar UStaticMeshComponent"));
        return nullptr;
    }

    // Configurar mesh
    MeshComp->SetStaticMesh(Mesh);
    
    // Configurações padrão
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetCastShadow(true);
    
    // Registrar no mundo
    MeshComp->RegisterComponent();

    UE_LOG(LogTemp, Log, TEXT("UEquippedItem::CreateMeshComponent - Componente visual criado para %s"), 
        *SourceItem.ItemData->ItemName.ToString());

    return MeshComp;
}