// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Actor/RPGProjectile.h"
#include "RPGAbilityTypes.h"
#include "RPGFireBall.generated.h"

/**
 * Projétil específico para habilidades de fogo que retorna ao jogador
 */
UCLASS()
class RPG_API ARPGFireBall : public ARPGProjectile
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintImplementableEvent)
	void StartOutgoingTimeline();

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> ReturnToActor;

	UPROPERTY(BlueprintReadWrite)
	FDamageEffectParams ExplosionDamageParams;
	
protected:
	virtual void BeginPlay() override;
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void OnHit() override;
	
	// Função para verificar se o overlap é válido
	bool IsValidOverlap(AActor* OtherActor);
}; 