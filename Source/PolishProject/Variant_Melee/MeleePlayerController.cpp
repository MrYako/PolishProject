#include "MeleePlayerController.h"

AMeleePlayerController::AMeleePlayerController()
{
	bShowMouseCursor = false;
}

void AMeleePlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetInputMode(FInputModeGameOnly());
}

void AMeleePlayerController::PlayerTick(float DeltaTime)
{
	float DX, DY;
	GetInputMouseDelta(DX, DY);

	if (!FMath::IsNearlyZero(DX))
	{
		FRotator Rot = GetControlRotation();
		Rot.Yaw += DX * MouseSensitivity;
		SetControlRotation(Rot);
	}

	Super::PlayerTick(DeltaTime);
}
