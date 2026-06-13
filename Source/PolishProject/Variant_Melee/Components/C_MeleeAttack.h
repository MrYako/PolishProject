#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "C_MeleeAttack.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeHit, AActor*, HitActor);

UCLASS(ClassGroup=(Melee), meta=(BlueprintSpawnableComponent))
class POLISHPROJECT_API UC_MeleeAttack : public USceneComponent
{
	GENERATED_BODY()

public:

	UC_MeleeAttack();

protected:

	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category="Combat")
	void TriggerAttack();

	void SetHitBox(UBoxComponent* Box);
	
	bool CanAttack() const { return bCanAttack && HitBox; }

	UPROPERTY(BlueprintAssignable)
	FOnMeleeHit OnHit;

private:

	TObjectPtr<UBoxComponent> HitBox;

	UPROPERTY(EditAnywhere, Category="Combat", meta=(ClampMin=0))
	float AttackDamage = 25.f;

	/** 0 = push purely away from attacker. 1 = push purely perpendicular to swing arc.
	    0.4 gives a natural fan when hitting multiple enemies. */
	UPROPERTY(EditAnywhere, Category="Combat", meta=(ClampMin=0, ClampMax=1))
	float KnockbackFanBlend = 0.4f;

	UPROPERTY(EditAnywhere, Category="Combat", meta=(ClampMin=0, Units="s"))
	float HitBoxActiveDuration = 0.3f;

	UPROPERTY(EditAnywhere, Category="Combat", meta=(ClampMin=0, Units="s"))
	float AttackCooldown = 0.6f;

	bool bCanAttack = true;
	FTimerHandle CooldownTimer;
	TSet<TObjectPtr<AActor>> HitActorsThisSwing;

	void EndAttack();

	UFUNCTION()
	void OnHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
