#include "UI/Characters/Party/PartyMemberStatusWidget.h"

#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Character/RPGCharacter.h"
#include "Character/PlayerClassInfo.h"
#include "Game/RPGGameModeBase.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
void UPartyMemberStatusWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Garantir que os widgets foram encontrados
    if (!HealthBar || !NameText || !HealthText || !ClassIcon)
    {
        UE_LOG(LogTemp, Warning, TEXT("PartyMemberStatusWidget: Alguns widgets não foram encontrados!"));
    }
}

void UPartyMemberStatusWidget::NativeDestruct()
{
    UnbindDelegates();
    
    // Limpar referências
    MemberCharacter.Reset();
    
    Super::NativeDestruct();
}

void UPartyMemberStatusWidget::SetupMember(ARPGCharacter* Character)
{
    // Primeiro, limpar delegates antigos
    UnbindDelegates();
    if (!Character)
    {
        return;
    }
    // Atualizar referências usando TWeakObjectPtr
    MemberCharacter = Character;
    // Bind de saúde via GAS
    if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
    {
        ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetHealthAttribute()).AddUObject(this, &UPartyMemberStatusWidget::OnHealthChangedFromGAS);
        float Current = ASC->GetNumericAttribute(URPGAttributeSet::GetHealthAttribute());
        float Max = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxHealthAttribute());
        UpdateHealth(Current, Max);
    }

    // Bind de mudança de classe
    Character->OnPlayerClassChanged.AddDynamic(this, &UPartyMemberStatusWidget::OnPlayerClassChanged);

    // Atualizar UI com valores iniciais
    UpdateName(Character->GetCharacterName());
    UpdateClassIcon();
}

void UPartyMemberStatusWidget::OnHealthChanged(float NewHealth, float MaxHealth)
{
    if (!MemberCharacter.IsValid())
        return;
        
    UpdateHealth(NewHealth, MaxHealth);
}

void UPartyMemberStatusWidget::OnHealthChangedFromGAS(const FOnAttributeChangeData& Data)
{
    if (!MemberCharacter.IsValid()) return;
    float Current = Data.NewValue;
    if (UAbilitySystemComponent* ASC = MemberCharacter.Get()->GetAbilitySystemComponent())
    {
        float Max = ASC->GetNumericAttribute(URPGAttributeSet::GetMaxHealthAttribute());
        UpdateHealth(Current, Max);
    }
}

void UPartyMemberStatusWidget::OnPlayerClassChanged(EPlayerClass NewClass)
{
    if (MemberCharacter.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("Classe do personagem %s alterada para: %d"), 
            *MemberCharacter.Get()->GetCharacterName(), (int32)NewClass);
        UpdateClassIcon();
    }
}

void UPartyMemberStatusWidget::UpdateHealth(float Current, float Max)
{
    // Atualizar barra de vida
    if (HealthBar)
    {
        float HealthPercent = Max > 0.0f ? (Current / Max) : 0.0f;
        HealthBar->SetPercent(HealthPercent);
    }
    
    // Atualizar texto de vida
    if (HealthText)
    {
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f/%.0f"), Current, Max)));
    }
}

void UPartyMemberStatusWidget::UpdateName(const FString& CharacterName)
{
    if (NameText)
    {
        NameText->SetText(FText::FromString(CharacterName));
    }
}

void UPartyMemberStatusWidget::UpdateClassIcon()
{
    if (!MemberCharacter.IsValid() || !ClassIcon)
    {
        return;
    }

    ARPGCharacter* Character = MemberCharacter.Get();
    if (!Character)
    {
        return;
    }

    // Obter GameMode
    ARPGGameModeBase* GameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));
    if (!GameMode || !GameMode->PlayerClassInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateClassIcon: GameMode ou PlayerClassInfo não encontrado!"));
        return;
    }

    // Obter PlayerClass do personagem
    EPlayerClass PlayerClass = Character->GetPlayerClass();
    
    // Buscar dados da classe
    FPlayerClassData ClassData = GameMode->PlayerClassInfo->GetPlayerClassInfo(PlayerClass);
    
    // Definir ícone
    if (ClassData.ClassIcon)
    {
        ClassIcon->SetBrushFromTexture(ClassData.ClassIcon);
        UE_LOG(LogTemp, Log, TEXT("UpdateClassIcon: Ícone definido para classe %d do personagem %s"), 
            (int32)PlayerClass, *Character->GetCharacterName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateClassIcon: Ícone não encontrado para classe %d do personagem %s"), 
            (int32)PlayerClass, *Character->GetCharacterName());
        ClassIcon->SetBrushFromTexture(nullptr);
    }
}

void UPartyMemberStatusWidget::UnbindDelegates()
{
    if (MemberCharacter.IsValid())
    {
        // Remover delegate de saúde
        if (UAbilitySystemComponent* ASC = MemberCharacter.Get()->GetAbilitySystemComponent())
        {
            ASC->GetGameplayAttributeValueChangeDelegate(URPGAttributeSet::GetHealthAttribute()).RemoveAll(this);
        }

        // Remover delegate de classe
        MemberCharacter.Get()->OnPlayerClassChanged.RemoveAll(this);
    }
    MemberCharacter.Reset();
} 
