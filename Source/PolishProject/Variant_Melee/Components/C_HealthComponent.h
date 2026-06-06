#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "C_HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageReceived, float, Damage, AActor*, DamageInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS(ClassGroup=(Melee), meta=(BlueprintSpawnableComponent))
class POLISHPROJECT_API UC_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UC_HealthComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health", meta=(ClampMin=1))
	float MaxHealth = 100.f;

	UFUNCTION(BlueprintCallable, Category="Health")
	void ApplyDamage(float Amount, AActor* DamageInstigator);

	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category="Health")
	bool IsAlive() const { return CurrentHealth > 0.f; }

	UPROPERTY(BlueprintAssignable)
	FOnDamageReceived OnDamageReceived;

	UPROPERTY(BlueprintAssignable)
	FOnDeath OnDeath;

protected:

	virtual void BeginPlay() override;

private:

	float CurrentHealth = 0.f;
};