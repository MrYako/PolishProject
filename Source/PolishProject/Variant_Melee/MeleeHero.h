#pragma once

#include "CoreMinimal.h"
#include "MeleeCharacterBase.h"
#include "MeleeHero.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS(abstract)
class POLISHPROJECT_API AMeleeHero : public AMeleeCharacterBase
{
	GENERATED_BODY()

private:

	UPROPERTY(VisibleAnywhere, Category="Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category="Camera")
	TObjectPtr<UCameraComponent> Camera;

protected:

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> AttackAction;

public:

	AMeleeHero();

protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void OnDeath_Implementation() override;
	virtual void ResetCharacter() override;

	/** World XY position the hero glides toward after death. */
	UPROPERTY(EditAnywhere, Category="Death")
	FVector2D GlideTargetLocation = FVector2D::ZeroVector;

	/** How long the glide takes — must match the respawn timer in GameMode. */
	UPROPERTY(EditAnywhere, Category="Death")
	float GlideDuration = 3.f;

private:

	float GlideSpeed = 0.f;

	void UpdateDeathGlide(float DeltaTime);
	void Move(const FInputActionValue& Value);
};
