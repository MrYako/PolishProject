#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatantInterface.h"
#include "Components/C_KnockbackComponent.h"
#include "MeleeCharacterBase.generated.h"

class UC_HealthComponent;
class UC_MeleeAttack;
class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeCharacterDied, AMeleeCharacterBase*, Character);

UCLASS(abstract)
class POLISHPROJECT_API AMeleeCharacterBase : public ACharacter, public ICombatantInterface
{
	GENERATED_BODY()

public:

	AMeleeCharacterBase();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Combat")
	void ReceiveDamage(float Amount, AActor* DamageCauser);

	/** ICombatantInterface */
	virtual void PerformAttack_Implementation() override;

	/** Thin wrapper — delegates to KnockbackComponent->Apply(). */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void ApplyKnockback(FVector Direction, int32 ChainDepth = 0);

	/** Hides the character and disables its collision and movement. */
	void Disable() const;

	/** Reverses OnDeath: restores health, re-enables collision/movement/meshes, restarts BT. */
	virtual void ResetCharacter();

	UPROPERTY(BlueprintAssignable, Category="Combat")
	FOnMeleeCharacterDied OnCharacterDied;

	UFUNCTION(BlueprintPure, Category="Components")
	UC_HealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category="Components")
	UC_KnockbackComponent* GetKnockbackComponent() const { return KnockbackComponent; }

protected:

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UC_KnockbackComponent> KnockbackComponent;

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleDamageReceived(float Damage, AActor* DamageInstigator);

	virtual void PostInitProperties() override;
};
