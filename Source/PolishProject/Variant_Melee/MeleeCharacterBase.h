#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MeleeCharacterBase.generated.h"

class UC_HealthComponent;
class UC_MeleeAttack;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS(abstract)
class POLISHPROJECT_API AMeleeCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:

	AMeleeCharacterBase();

	UFUNCTION(BlueprintCallable, Category="Combat")
	void ReceiveDamage(float Amount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category="Combat")
	void PerformAttack();

	/** Launches the character in Direction with Strength. Used for hit knockback. */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void ApplyKnockback(FVector Direction, float Strength);

protected:

	virtual void BeginPlay() override;

	/** C++ default: disables collision and stops movement. Override in BP to play death anim/effects. */
	UFUNCTION(BlueprintNativeEvent, Category="Combat")
	void OnDeath();
	virtual void OnDeath_Implementation();

	/** No C++ default. Override in BP for hit flash, sound, etc. */
	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void OnDamageReceived(float Damage, AActor* DamageInstigator);

	/** No C++ default. Override in BP to animate weapon. */
	UFUNCTION(BlueprintImplementableEvent, Category="Combat")
	void PlayAttackAnimation();

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> HitBox;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UC_MeleeAttack> AttackComponent;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UC_HealthComponent> HealthComponent;

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleDamageReceived(float Damage, AActor* DamageInstigator);
	
	void PostInitProperties() override;
};