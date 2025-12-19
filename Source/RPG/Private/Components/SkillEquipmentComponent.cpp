// Copyright Druid Mechanics

#include "Components/SkillEquipmentComponent.h"
#include "Character/RPGCharacter.h"
#include "Progression/SkillTreeSubsystem.h"
#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/Base/RPGGameplayAbility.h"
#include "Game/RPGGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Progression/SkillTreeTableRow.h"

USkillEquipmentComponent::USkillEquipmentComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USkillEquipmentComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Conectar aos eventos do SkillTreeSubsystem
    ConnectToSkillTreeEvents();
}

void USkillEquipmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// === IDENTIFICAÇÃO ===

ARPGCharacter* USkillEquipmentComponent::GetOwnerCharacter() const
{
    return Cast<ARPGCharacter>(GetOwner());
}

FName USkillEquipmentComponent::GetCharacterID() const
{
    if (ARPGCharacter* Character = GetOwnerCharacter())
    {
        return Character->GetCharacterUniqueID();
    }
    return NAME_None;
}

// === EQUIPAMENTO ===

bool USkillEquipmentComponent::CanEquipSkill(const FName& SkillID, const FName& SlotID) const
{
    if (SkillID.IsNone() || SlotID.IsNone())
    {
        return false;
    }
    
    ARPGCharacter* Character = GetOwnerCharacter();
    if (!Character)
    {
        return false;
    }
    
    // Verificar se o slot já tem uma habilidade equipada (PERMITIR para auto-remover)
    if (EquippedSkills.Contains(SlotID))
    {
        // ✅ PERMITIR (auto-remover vai lidar com isso)
    }
    
    // Verificar se a habilidade já está equipada em outro slot
    if (IsSkillEquipped(SkillID))
    {
        return false;
    }
    
    // Verificar se a habilidade está desbloqueada no SkillTreeSubsystem
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
            {
                if (!SkillTreeSubsystem->SkillUnlocked(Character, SkillID))
                {
                    return false;
                }
            }
        }
    }
    
    return true;
}

bool USkillEquipmentComponent::EquipSkill(const FName& SkillID, const FName& SlotID, const FGameplayTag& SlotInputTag)
{
    if (!CanEquipSkill(SkillID, SlotID))
    {
        return false;
    }
    
    // Auto-remover: Desequipar habilidade atual no slot (se houver)
    if (HasSlot(SlotID))
    {
        // Desequipar habilidade atual
        UnequipSkill(SlotID);
    }
    
    // Equipar a nova habilidade no slot
    EquippedSkills.Add(SlotID, SkillID);
    
    // ✅ NOVO: Conceder via GAS com a InputTag do slot
    GrantSkillToCharacter(SkillID, SlotInputTag);
    
    // Disparar delegates
    FName CharacterID = GetCharacterID();
    OnSkillEquipmentEquipped.Broadcast(CharacterID, SlotID, SkillID);
    
    // ✅ NOVO: Disparar delegate com ícone
    UTexture2D* SkillIcon = GetSkillIcon(SkillID);
    OnSkillEquippedInSlot.Broadcast(SlotID, SkillID, SkillIcon);
    
    return true;
}

bool USkillEquipmentComponent::UnequipSkill(const FName& SlotID)
{
    if (SlotID.IsNone())
    {
        return false;
    }
    
    // Verificar se o slot tem uma habilidade equipada
    FName* SkillIDPtr = EquippedSkills.Find(SlotID);
    if (!SkillIDPtr)
    {
        return false;
    }
    
    FName SkillIDToRemove = *SkillIDPtr;
    
    // Remover via GAS
    RemoveSkillFromCharacter(SkillIDToRemove);
    
    // Remover do mapa (remove o slot também)
    EquippedSkills.Remove(SlotID);
    
    // Disparar delegate
    FName CharacterID = GetCharacterID();
    OnSkillEquipmentUnequipped.Broadcast(CharacterID, SlotID, SkillIDToRemove);
    
    return true;
}

void USkillEquipmentComponent::UnequipAllSkills()
{
    // Coletar todos os SlotIDs para evitar modificar o mapa durante iteração
    TArray<FName> SlotsToUnequip;
    EquippedSkills.GetKeys(SlotsToUnequip);
    
    // Desequipar cada slot
    for (const FName& SlotID : SlotsToUnequip)
    {
        UnequipSkill(SlotID);
    }
}

bool USkillEquipmentComponent::IsSkillEquipped(const FName& SkillID) const
{
    if (SkillID.IsNone())
    {
        return false;
    }
    
    // Procurar a habilidade em todos os slots
    for (const auto& Pair : EquippedSkills)
    {
        if (Pair.Value == SkillID)
        {
            return true;
        }
    }
    
    return false;
}

bool USkillEquipmentComponent::IsSkillEquippedInSlot(const FName& SkillID, const FName& SlotID) const
{
    if (SkillID.IsNone() || SlotID.IsNone())
    {
        return false;
    }
    
    const FName* EquippedSkillPtr = EquippedSkills.Find(SlotID);
    return EquippedSkillPtr && (*EquippedSkillPtr == SkillID);
}

FName USkillEquipmentComponent::GetSkillEquippedSlot(const FName& SkillID) const
{
    if (SkillID.IsNone())
    {
        return NAME_None;
    }
    
    // Procurar a habilidade em todos os slots
    for (const auto& Pair : EquippedSkills)
    {
        if (Pair.Value == SkillID)
        {
            return Pair.Key; // Retorna o SlotID
        }
    }
    
    return NAME_None;
}

FName USkillEquipmentComponent::GetEquippedSkillInSlot(const FName& SlotID) const
{
    if (SlotID.IsNone())
    {
        return NAME_None;
    }
    
    return EquippedSkills.FindRef(SlotID); // Retorna NAME_None se não encontrar
}

TMap<FName, FName> USkillEquipmentComponent::GetAllEquippedSkills() const
{
    return EquippedSkills;
}

bool USkillEquipmentComponent::HasSlot(const FName& SlotID) const
{
    return EquippedSkills.Contains(SlotID);
}

TArray<FName> USkillEquipmentComponent::GetActiveSlotIDs() const
{
    TArray<FName> ActiveSlots;
    EquippedSkills.GetKeys(ActiveSlots);
    return ActiveSlots;
}

bool USkillEquipmentComponent::MoveSkillToSlot(const FName& FromSlotID, const FName& ToSlotID)
{
    if (FromSlotID.IsNone() || ToSlotID.IsNone() || FromSlotID == ToSlotID)
    {
        return false;
    }
    
    // Verificar se o slot de origem tem uma habilidade
    FName* SkillToMovePtr = EquippedSkills.Find(FromSlotID);
    if (!SkillToMovePtr)
    {
        return false;
    }
    
    FName SkillToMove = *SkillToMovePtr;
    
    // Verificar se o slot de destino já tem uma habilidade (nesse caso, fazer swap)
    if (EquippedSkills.Contains(ToSlotID))
    {
        return SwapSkillsBetweenSlots(FromSlotID, ToSlotID);
    }
    
    // Desequipar habilidade do slot de origem
    if (!UnequipSkill(FromSlotID))
    {
        return false;
    }
    
    // Equipar habilidade no slot de destino
    if (!EquipSkill(SkillToMove, ToSlotID, FGameplayTag::EmptyTag))
    {
        // Reverter se falhar
        EquipSkill(SkillToMove, FromSlotID, FGameplayTag::EmptyTag);
        return false;
    }
    
    return true;
}

bool USkillEquipmentComponent::SwapSkillsBetweenSlots(const FName& SlotID1, const FName& SlotID2)
{
    if (SlotID1.IsNone() || SlotID2.IsNone() || SlotID1 == SlotID2)
    {
        return false;
    }
    
    FName Skill1 = GetEquippedSkillInSlot(SlotID1);
    FName Skill2 = GetEquippedSkillInSlot(SlotID2);
    
    // Pelo menos um slot deve ter uma habilidade
    if (Skill1.IsNone() && Skill2.IsNone())
    {
        return false;
    }
    
    // Desequipar ambas as habilidades
    if (!Skill1.IsNone()) UnequipSkill(SlotID1);
    if (!Skill2.IsNone()) UnequipSkill(SlotID2);
    
    bool bSuccess = true;
    
    // Equipar as habilidades nos slots trocados
    if (!Skill1.IsNone() && !EquipSkill(Skill1, SlotID2, FGameplayTag::EmptyTag))
    {
        bSuccess = false;
    }
    
    if (!Skill2.IsNone() && !EquipSkill(Skill2, SlotID1, FGameplayTag::EmptyTag))
    {
        bSuccess = false;
    }
    
    // Reverter se houve falha
    if (!bSuccess)
    {
        if (!Skill1.IsNone()) EquipSkill(Skill1, SlotID1, FGameplayTag::EmptyTag);
        if (!Skill2.IsNone()) EquipSkill(Skill2, SlotID2, FGameplayTag::EmptyTag);
        return false;
    }
    
    return true;
}

// === HELPERS ===

void USkillEquipmentComponent::ValidateWithSkillTreeSubsystem()
{
    // Validar todas as habilidades equipadas
    TArray<FName> InvalidSlots;
    
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
            {
                ARPGCharacter* Character = GetOwnerCharacter();
                if (Character)
                {
                    for (const auto& Pair : EquippedSkills)
                    {
                        if (!SkillTreeSubsystem->SkillUnlocked(Character, Pair.Value))
                        {
                            InvalidSlots.Add(Pair.Key);
                        }
                    }
                }
            }
        }
    }
    
    // Remover habilidades inválidas
    for (const FName& SlotID : InvalidSlots)
    {
        UnequipSkill(SlotID);
    }
}

void USkillEquipmentComponent::GrantSkillToCharacter(const FName& SkillID, const FGameplayTag& SlotInputTag)
{
    ARPGCharacter* Character = GetOwnerCharacter();
    if (!Character)
    {
        return;
    }
    
    // Obter a classe da habilidade via SkillTreeSubsystem
    TSubclassOf<UGameplayAbility> AbilityClass = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
            {
                // Buscar dados da habilidade no Data Table
                FName CharacterID = Character->GetCharacterUniqueID();
                UDataTable* DataTable = SkillTreeSubsystem->GetCharacterSkillTable(CharacterID);
                
                if (DataTable)
                {
                    TArray<FName> RowNames = DataTable->GetRowNames();
                    for (const FName& RowName : RowNames)
                    {
                        if (FSkillTreeTableRow* Row = DataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GrantSkillToCharacter")))
                        {
                            if (Row->SkillID == SkillID)
                            {
                                AbilityClass = Row->AbilityClass;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Conceder a habilidade via GAS
    if (AbilityClass && Character->GetAbilitySystemComponent())
    {
        // Criar spec da habilidade
        FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
        
        // ✅ NOVO: Usar a InputTag do slot em vez da da habilidade
        if (!SlotInputTag.IsValid())
        {
            // Fallback: usar tag da habilidade se a do slot for inválida
            if (const URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(AbilitySpec.Ability))
            {
                AbilitySpec.GetDynamicSpecSourceTags().AddTag(RPGAbility->StartupInputTag);
            }
        }
        else
        {
            // ✅ NOVO: Configurar a tag de input do slot
            AbilitySpec.GetDynamicSpecSourceTags().AddTag(SlotInputTag);
        }
        
        // Conceder a habilidade
        Character->GetAbilitySystemComponent()->GiveAbility(AbilitySpec);
    }
}

void USkillEquipmentComponent::RemoveSkillFromCharacter(const FName& SkillID)
{
    ARPGCharacter* Character = GetOwnerCharacter();
    if (!Character)
    {
        return;
    }
    
    // Obter a classe da habilidade via SkillTreeSubsystem
    TSubclassOf<UGameplayAbility> AbilityClass = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
            {
                // Buscar dados da habilidade no Data Table
                FName CharacterID = Character->GetCharacterUniqueID();
                UDataTable* DataTable = SkillTreeSubsystem->GetCharacterSkillTable(CharacterID);
                
                if (DataTable)
                {
                    TArray<FName> RowNames = DataTable->GetRowNames();
                    for (const FName& RowName : RowNames)
                    {
                        if (FSkillTreeTableRow* Row = DataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("RemoveSkillFromCharacter")))
                        {
                            if (Row->SkillID == SkillID)
                            {
                                AbilityClass = Row->AbilityClass;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Remover a habilidade via GAS
    if (AbilityClass && Character->GetAbilitySystemComponent())
    {
        UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
        TArray<FGameplayAbilitySpec> AbilitySpecs = ASC->GetActivatableAbilities();
        
        for (const FGameplayAbilitySpec& Spec : AbilitySpecs)
        {
            if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
            {
                ASC->ClearAbility(Spec.Handle);
                break;
            }
        }
    }
}

// === EVENTOS ===

void USkillEquipmentComponent::ConnectToSkillTreeEvents()
{
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
            {
                // Conectar ao evento de habilidade desbloqueada
                SkillTreeSubsystem->OnSkillUnlocked.AddDynamic(this, &USkillEquipmentComponent::OnSkillUnlocked);
            }
        }
    }
}

void USkillEquipmentComponent::OnSkillUnlocked(ARPGCharacter* Character, FName SkillID)
{
    // Verificar se é o nosso personagem
    ARPGCharacter* OwnerCharacter = GetOwnerCharacter();
    if (Character != OwnerCharacter)
    {
        return; // Não é nosso personagem
    }
    
    // Aqui você pode adicionar lógica adicional quando uma habilidade é desbloqueada
    // Por exemplo: mostrar notificação, atualizar UI, etc.
}

UTexture2D* USkillEquipmentComponent::GetSkillIcon(const FName& SkillID) const
{
    if (SkillID.IsNone())
    {
        return nullptr;
    }
    
    // Buscar ícone da habilidade via SkillTreeSubsystem
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
            {
                ARPGCharacter* Character = GetOwnerCharacter();
                if (Character)
                {
                    FName CharacterID = Character->GetCharacterUniqueID();
                    
                    // Buscar dados da habilidade na tabela
                    UDataTable* CharacterDataTable = SkillTreeSubsystem->GetCharacterSkillTable(CharacterID);
                    if (CharacterDataTable)
                    {
                        TArray<FName> RowNames = CharacterDataTable->GetRowNames();
                        
                        for (const FName& RowName : RowNames)
                        {
                            if (FSkillTreeTableRow* Row = CharacterDataTable->FindRow<FSkillTreeTableRow>(RowName, TEXT("GetSkillIcon")))
                            {
                                if (Row->SkillID == SkillID)
                                {
                                    return Row->SkillIcon;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return nullptr;
}

UTexture2D* USkillEquipmentComponent::GetDefaultSlotIcon() const
{
    // Carregar IconClear específico
    UTexture2D* ClearIcon = LoadObject<UTexture2D>(nullptr, TEXT("/Game/RPG/Blueprints/UI/Character/SkillIconSlot/IconClear"));
    return ClearIcon;
}

UTexture2D* USkillEquipmentComponent::GetSlotIcon(const FName& SlotID) const
{
    // Obter habilidade equipada no slot
    FName EquippedSkillID = GetEquippedSkillInSlot(SlotID);
    
    if (!EquippedSkillID.IsNone())
    {
        // Buscar ícone da habilidade equipada
        UTexture2D* SkillIcon = GetSkillIcon(EquippedSkillID);
        
        if (SkillIcon)
        {
            return SkillIcon;
        }
    }
    
    // Se não há habilidade equipada, retornar ícone padrão
    return GetDefaultSlotIcon();
}

