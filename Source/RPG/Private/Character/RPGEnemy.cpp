// Copyright Druid Mechanics


#include "Character/RPGEnemy.h"

#include "AbilitySystem/Core/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Core/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/Core/RPGAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "RPG/RPG.h"
#include "UI/Widget/RPGUserWidget.h"
#include "RPGGameplayTags.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/RPGAIController.h"
#include "UI/Characters/Enemy/EnemyHealthUserWidget.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Party/PartySubsystem.h"


ARPGEnemy::ARPGEnemy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Team = ERPGTeam::Enemy;

	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(GetRootComponent());
	HealthBar->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBar->SetDrawSize(FVector2D(150.f, 30.f));
	if (HealthBarWidgetClass)
	{
		HealthBar->SetWidgetClass(HealthBarWidgetClass);
	}
	const float ZOffset = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 20.f;
	HealthBar->SetRelativeLocation(FVector(0.f, 0.f, ZOffset));
	HealthBar->SetPivot(FVector2D(0.5f, 0.f));

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->MarkRenderStateDirty();
	
	// Configurar propriedades herdadas do CharacterBase
	BaseWalkSpeed = 250.f;
	CharacterClass = ECharacterClass::Warrior;
}

void ARPGEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!NewController)
	{
		return;
	}

	if (!HasAuthority()) 
	{
		return;
	}

	RPGAIController = Cast<ARPGAIController>(NewController);
	if (RPGAIController && BehaviorTree)
	{
		RPGAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
		RPGAIController->RunBehaviorTree(BehaviorTree);
		RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	}
}



int32 ARPGEnemy::GetCharacterLevel_Implementation() const
{
	return Level;
}

ECharacterClass ARPGEnemy::GetCharacterClass_Implementation()
{
	return CharacterClass;
}

void ARPGEnemy::Die(const FVector& DeathImpulse)
{
	// Definir a chave "Dead" no Blackboard como true
	if (RPGAIController && RPGAIController->GetBlackboardComponent())
	{
		RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
	}
	
			// Sistema de missões agora é tratado em SendXPEvent() no AttributeSet
	
	SetLifeSpan(LifeSpan);
	SpawnLoot();
	Super::Die(DeathImpulse);
}

// Override de MulticastHandleDeath apenas para dissolver arma do inimigo
void ARPGEnemy::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	Super::MulticastHandleDeath_Implementation(DeathImpulse);
	
	// Esconder a HealthBar quando o inimigo morrer
	if (HealthBar)
	{
		HealthBar->SetVisibility(false);
	}
}

void ARPGEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* ARPGEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void ARPGEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	if (HasAuthority())
	{
		URPGAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);	
	}

	
	if (UEnemyHealthUserWidget* HealthWidget = Cast<UEnemyHealthUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		HealthWidget->SetEnemyController(this);
	}
	
	if (const URPGAttributeSet* RPGAS = Cast<URPGAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RPGAS->GetHealthAttribute()).AddLambda(
			[this, RPGAS](const FOnAttributeChangeData& Data)
			{
				// Broadcast current health and max health
				OnHealthChanged.Broadcast(Data.NewValue, RPGAS->GetMaxHealth());
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RPGAS->GetMaxHealthAttribute()).AddLambda(
			[this, RPGAS](const FOnAttributeChangeData& Data)
			{
				// Broadcast current health and new max health
				OnMaxHealthChanged.Broadcast(RPGAS->GetHealth(), Data.NewValue);
			}
		);
		
		AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this,
			&ARPGEnemy::HitReactTagChanged
		);

		{
			// Broadcast initial health values
			float CurrentHealth = RPGAS->GetHealth();
			float MaxHealth = RPGAS->GetMaxHealth();
			OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
			OnMaxHealthChanged.Broadcast(CurrentHealth, MaxHealth);
		}
	}
	
}

void ARPGEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;

	if (RPGAIController && RPGAIController->GetBlackboardComponent())
	{
		RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void ARPGEnemy::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();
	
	if (!AbilitySystemComponent)
	{
		return;
	}

	URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent);
	if (RPGASC)
	{
		RPGASC->AbilityActorInfoSet();
	}

	RegisterGameplayTagEvents();
	
	if (HasAuthority())
	{
		InitializeDefaultAttributes();
	}
	
	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

void ARPGEnemy::RegisterGameplayTagEvents()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ARPGEnemy::StunTagChanged);
}

void ARPGEnemy::InitializeDefaultAttributes() const
{
    // Inicialização de atributos baseada no Level local
    URPGAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void ARPGEnemy::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::StunTagChanged(CallbackTag, NewCount);
}


