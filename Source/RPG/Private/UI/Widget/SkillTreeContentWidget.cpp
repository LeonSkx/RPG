// Copyright Druid Mechanics

#include "UI/Widget/SkillTreeContentWidget.h"
#include "UI/WidgetController/SkillTreeWidgetController.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/ScrollBox.h"
#include "Components/ComboBoxString.h"
#include "Party/PartySubsystem.h"
#include "Character/RPGCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Progression/SkillTreeSubsystem.h"

void USkillTreeContentWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            WidgetController = NewObject<USkillTreeWidgetController>(this);

            if (WidgetController)
            {
                if (CharacterSelector)
                {
                    CharacterSelector->OnSelectionChanged.AddDynamic(this, &USkillTreeContentWidget::OnCharacterSelectionChanged);
                    InitializeCharacterSelector();
                }

                if (UPartySubsystem* PartySubsystem = GameInstance->GetSubsystem<UPartySubsystem>())
                {
                    ARPGCharacter* ActiveCharacter = PartySubsystem->GetActiveCharacter();

                    if (ActiveCharacter)
                    {
                        CurrentCharacter = ActiveCharacter;
                        WidgetController->InitializeWithCharacter(ActiveCharacter);
                        InitializeSkillTree();

                        if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
                        {
                            SkillTreeSubsystem->OnPointsChanged.AddDynamic(this, &USkillTreeContentWidget::OnPointsChanged);
                        }
                    }
                }
            }
        }
    }

    SetupPan();
}

void USkillTreeContentWidget::NativeDestruct()
{
    if (CharacterSelector)
    {
        CharacterSelector->OnSelectionChanged.RemoveAll(this);
    }

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            if (USkillTreeSubsystem* SkillTreeSubsystem = GameInstance->GetSubsystem<USkillTreeSubsystem>())
            {
                SkillTreeSubsystem->OnPointsChanged.RemoveAll(this);
            }
        }
    }

    Super::NativeDestruct();

    WidgetController = nullptr;
    CurrentCharacter = nullptr;
    CharacterMap.Empty();
}

void USkillTreeContentWidget::SetWidgetController(USkillTreeWidgetController* InWidgetController)
{
    WidgetController = InWidgetController;

    if (WidgetController)
    {
        UpdateAllNodes();
    }
}

void USkillTreeContentWidget::InitializeSkillTree()
{
    if (!WidgetController) return;

    UpdateHeader();
}

void USkillTreeContentWidget::UpdateHeader()
{
    if (!WidgetController) return;

    if (CharacterNameText)
    {
        FString CharacterName = WidgetController->GetCharacterName();
        CharacterNameText->SetText(FText::FromString(CharacterName));
    }

    if (CharacterLevelText)
    {
        int32 Level = WidgetController->GetCharacterLevel();
        FString LevelString = FString::Printf(TEXT("Nível %d"), Level);
        CharacterLevelText->SetText(FText::FromString(LevelString));
    }

    if (SpellPointsText)
    {
        int32 SpellPoints = WidgetController->GetSpellPoints();
        FString SpellPointsString = FString::Printf(TEXT("Pontos de Magia: %d"), SpellPoints);
        SpellPointsText->SetText(FText::FromString(SpellPointsString));
    }
}

void USkillTreeContentWidget::UpdateAllNodes()
{
    if (!WidgetController) return;

    UpdateHeader();

    if (CurrentCharacter)
    {
        FName CharacterID = CurrentCharacter->GetCharacterUniqueID();
        OnCharacterChanged.Broadcast(CharacterID);
    }
}

void USkillTreeContentWidget::InitializeCharacterSelector()
{
    if (!CharacterSelector) return;

    CharacterSelector->ClearOptions();
    CharacterMap.Empty();

    UPartySubsystem* PartySubsystem = GetGameInstance()->GetSubsystem<UPartySubsystem>();
    if (!PartySubsystem) return;

    ARPGCharacter* ActiveCharacter = PartySubsystem->GetActivePartyMember();
    TArray<ARPGCharacter*> PartyMembers = PartySubsystem->GetPartyMembers();

    TArray<ARPGCharacter*> OrderedPartyMembers;

    if (ActiveCharacter && PartyMembers.Contains(ActiveCharacter))
    {
        OrderedPartyMembers.Add(ActiveCharacter);
    }

    for (ARPGCharacter* Character : PartyMembers)
    {
        if (Character && Character != ActiveCharacter)
        {
            OrderedPartyMembers.Add(Character);
        }
    }

    for (ARPGCharacter* Character : OrderedPartyMembers)
    {
        if (Character)
        {
            FString CharacterName = Character->GetCharacterName();
            CharacterSelector->AddOption(CharacterName);
            CharacterMap.Add(CharacterName, Character);
        }
    }

    if (CharacterSelector->GetOptionCount() > 0)
    {
        CharacterSelector->SetSelectedIndex(0);
        FString SelectedOption = CharacterSelector->GetSelectedOption();
        ARPGCharacter* SelectedCharacter = CharacterMap.FindRef(SelectedOption);
        CurrentCharacter = SelectedCharacter;

        if (WidgetController)
        {
            WidgetController->InitializeWithCharacter(CurrentCharacter);
        }

        UpdateHeader();

        if (CurrentCharacter)
        {
            FName CharacterID = CurrentCharacter->GetCharacterUniqueID();
            OnCharacterChanged.Broadcast(CharacterID);
        }
    }
}

void USkillTreeContentWidget::OnCharacterSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    if (SelectionType != ESelectInfo::OnMouseClick && SelectionType != ESelectInfo::OnKeyPress)
    {
        return;
    }

    ARPGCharacter* SelectedCharacter = CharacterMap.FindRef(SelectedItem);
    if (SelectedCharacter)
    {
        CurrentCharacter = SelectedCharacter;

        if (WidgetController)
        {
            WidgetController->InitializeWithCharacter(CurrentCharacter);
        }

        FName CharacterID = SelectedCharacter->GetCharacterUniqueID();
        UpdateHeader();
        OnCharacterChanged.Broadcast(CharacterID);
    }
}

void USkillTreeContentWidget::OnPointsChanged()
{
    if (SpellPointsText && WidgetController)
    {
        int32 SpellPoints = WidgetController->GetSpellPoints();
        FString SpellPointsString = FString::Printf(TEXT("Pontos de Magia: %d"), SpellPoints);
        SpellPointsText->SetText(FText::FromString(SpellPointsString));
    }
}

ARPGCharacter* USkillTreeContentWidget::GetCharacterByID(const FName& CharacterID) const
{
    for (const auto& Pair : CharacterMap)
    {
        if (Pair.Value && Pair.Value->GetCharacterUniqueID() == CharacterID)
        {
            return Pair.Value;
        }
    }
    return nullptr;
}

void USkillTreeContentWidget::SetupPan()
{
    if (!bPanEnabled || !TreeContainer)
    {
        return;
    }
    
    // Inicializar zoom
    CurrentZoom = DefaultZoom;
    TargetZoom = DefaultZoom;
    bIsSmoothZooming = false;
    
    // As funções NativeOnMouseButtonDown, NativeOnMouseMove e NativeOnMouseButtonUp são funções virtuais
    // da classe UUserWidget, não precisam de BindDynamic
}

FReply USkillTreeContentWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!bPanEnabled || !TreeContainer)
    {
        return FReply::Unhandled();
    }

    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bIsPanning = true;
        LastMousePosition = InMouseEvent.GetScreenSpacePosition();
        return FReply::Handled().CaptureMouse(TakeWidget());
    }

    return FReply::Unhandled();
}

FReply USkillTreeContentWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!bPanEnabled || !bIsPanning || !TreeContainer)
    {
        return FReply::Unhandled();
    }

    FVector2D CurrentMousePosition = InMouseEvent.GetScreenSpacePosition();
    FVector2D MouseDelta = CurrentMousePosition - LastMousePosition;

    CurrentPanOffset += MouseDelta * PanSpeed;

    ClampPanOffset(CurrentPanOffset);
    ApplyPanToContainer();

    LastMousePosition = CurrentMousePosition;

    return FReply::Handled();
}

FReply USkillTreeContentWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!bPanEnabled)
    {
        return FReply::Unhandled();
    }

    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bIsPanning = false;
        return FReply::Handled().ReleaseMouseCapture();
    }

    return FReply::Unhandled();
}

void USkillTreeContentWidget::ApplyPanToContainer()
{
    if (!TreeContainer) return;

    // Aplicar PAN e ZOOM ao container
    FWidgetTransform Transform;
    Transform.Translation = CurrentPanOffset;
    Transform.Scale = FVector2D(CurrentZoom, CurrentZoom);
    Transform.Angle = 0.0f;
    
    TreeContainer->SetRenderTransform(Transform);
}

void USkillTreeContentWidget::ClampPanOffset(FVector2D& Offset)
{
    Offset.X = FMath::Clamp(Offset.X, MinPanOffset.X, MaxPanOffset.X);
    Offset.Y = FMath::Clamp(Offset.Y, MinPanOffset.Y, MaxPanOffset.Y);
}

void USkillTreeContentWidget::ResetPanPosition()
{
    CurrentPanOffset = FVector2D::ZeroVector;
    ApplyPanToContainer();
}

void USkillTreeContentWidget::SetPanEnabled(bool bEnable)
{
    bPanEnabled = bEnable;

    if (!bEnable && bIsPanning)
    {
        bIsPanning = false;
        if (HasMouseCapture())
        {
            FSlateApplication::Get().ReleaseAllPointerCapture();
        }
    }
}

// === ZOOM SUAVE ===

void USkillTreeContentWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // Aplicar zoom suave se estiver ativo
    if (bSmoothZoomEnabled && bIsSmoothZooming)
    {
        // Interpolação suave entre CurrentZoom e TargetZoom
        float DeltaZoom = (TargetZoom - CurrentZoom) * SmoothZoomSpeed * InDeltaTime;
        
        // Se a diferença for muito pequena, finalizar
        if (FMath::Abs(DeltaZoom) < 0.001f)
        {
            CurrentZoom = TargetZoom;
            bIsSmoothZooming = false;
        }
        else
        {
            CurrentZoom += DeltaZoom;
        }
        
        // Aplicar ao container
        ApplyZoomToContainer();
    }
}

// === ZOOM ===

FReply USkillTreeContentWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!bZoomEnabled || !TreeContainer)
    {
        return FReply::Unhandled();
    }

    // Obter delta do scroll
    float WheelDelta = InMouseEvent.GetWheelDelta();
    
    // Calcular novo zoom alvo
    float NewTargetZoom = TargetZoom + (WheelDelta * ZoomSpeed);
    
    // Aplicar limites
    NewTargetZoom = FMath::Clamp(NewTargetZoom, MinZoom, MaxZoom);
    
    // Definir zoom alvo
    TargetZoom = NewTargetZoom;
    
    // Iniciar zoom suave
    if (bSmoothZoomEnabled)
    {
        bIsSmoothZooming = true;
    }
    else
    {
        // Zoom instantâneo se suave estiver desabilitado
        CurrentZoom = TargetZoom;
        ApplyZoomToContainer();
    }
    
    return FReply::Handled();
}

void USkillTreeContentWidget::ApplyZoomToContainer()
{
    if (!TreeContainer)
    {
        return;
    }

    // Aplicar zoom ao container
    FWidgetTransform Transform;
    Transform.Translation = CurrentPanOffset;
    Transform.Scale = FVector2D(CurrentZoom, CurrentZoom);
    Transform.Angle = 0.0f;
    
    TreeContainer->SetRenderTransform(Transform);
}

void USkillTreeContentWidget::ResetZoom()
{
    TargetZoom = DefaultZoom;
    
    if (bSmoothZoomEnabled)
    {
        bIsSmoothZooming = true;
    }
    else
    {
        CurrentZoom = DefaultZoom;
        ApplyZoomToContainer();
    }
}

void USkillTreeContentWidget::SetZoomEnabled(bool bEnable)
{
    bZoomEnabled = bEnable;
}
