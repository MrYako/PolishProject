#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_KnockbackComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKnockbackEvent);

UCLASS(ClassGroup=(Melee), meta=(BlueprintSpawnableComponent))
class POLISHPROJECT_API UC_KnockbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UC_KnockbackComponent();

	UFUNCTION(BlueprintCallable, Category="Combat|Knockback")
	void Apply(FVector Direction, int32 ChainDepth = 0);

	/** Cancels knockback immediately without broadcasting OnKnockbackEnded. Use on death/reset. */
	void Cancel();

	UFUNCTION(BlueprintPure, Category="Combat|Knockback")
	bool IsKnockedBack() const { return bIsKnockedBack; }

	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0", Units="cm"))
	float KnockbackDistance = 150.f;

	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0.01", Units="s"))
	float KnockbackDuration = 0.25f;

	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0", Units="s"))
	float KnockbackRecoveryTime = 0.10f;

	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0", ClampMax="3"))
	int32 MaxChainDepth = 1;

	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0", Units="cm"))
	float ChainPushDistance = 60.f;

	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0.01", Units="s"))
	float ChainPushDuration = 0.12f;

	UPROPERTY(BlueprintAssignable, Category="Combat|Knockback")
	FOnKnockbackEvent OnKnockbackStarted;

	UPROPERTY(BlueprintAssignable, Category="Combat|Knockback")
	FOnKnockbackEvent OnKnockbackEnded;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	bool bIsKnockedBack = false;
	int32 CurrentChainDepth = 0;
	FVector KnockbackStartPos = FVector::ZeroVector;
	FVector KnockbackTargetPos = FVector::ZeroVector;
	float KnockbackElapsed = 0.f;
	float KnockbackActiveDuration = 0.f;
	FTimerHandle KnockbackRecoveryTimer;

	void TickKnockback(float DeltaTime);
	void EndKnockback();
};
