#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CombatantInterface.h"
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

	UFUNCTION(BlueprintCallable, Category="Combat")
	void ApplyKnockback(FVector Direction, int32 ChainDepth = 0);

	virtual void Tick(float DeltaTime) override;

	/** Distance this character travels when knocked back (primary hit). */
	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0", Units="cm"))
	float KnockbackDistance = 150.f;

	/** Time (seconds) to travel KnockbackDistance. Controls apparent speed. */
	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0.01", Units="s"))
	float KnockbackDuration = 0.25f;

	/** Pause before BT restarts after knockback travel completes. */
	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0", Units="s"))
	float KnockbackRecoveryTime = 0.10f;

	/** Max chain levels. 0 = no chain. 1 = one neighbor push. */
	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0", ClampMax="3"))
	int32 MaxKnockbackChainDepth = 1;

	/** Distance for secondary (chain) pushes. */
	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0", Units="cm"))
	float ChainPushDistance = 60.f;

	/** Time to travel ChainPushDistance. */
	UPROPERTY(EditAnywhere, Category="Combat|Knockback", meta=(ClampMin="0.01", Units="s"))
	float ChainPushDuration = 0.12f;

	/** Hides the character and disables its collision and movement. */
	void Disable() const;

	/** Reverses OnDeath: restores health, re-enables collision/movement/meshes, restarts BT. */
	virtual void ResetCharacter();

	UPROPERTY(BlueprintAssignable, Category="Combat")
	FOnMeleeCharacterDied OnCharacterDied;

	UFUNCTION(BlueprintPure, Category="Components")
	UC_HealthComponent* GetHealthComponent() const { return HealthComponent; }

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

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleDamageReceived(float Damage, AActor* DamageInstigator);

	UFUNCTION()
	void HandleCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void PostInitProperties() override;

private:

	bool bIsKnockedBack = false;
	int32 CurrentKnockbackChainDepth = 0;
	FVector KnockbackStartPos = FVector::ZeroVector;
	FVector KnockbackTargetPos = FVector::ZeroVector;
	float KnockbackElapsed = 0.f;
	float KnockbackActiveDuration = 0.f;
	FTimerHandle KnockbackRecoveryTimer;

	void EndKnockback();
};