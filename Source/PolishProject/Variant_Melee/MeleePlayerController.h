#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MeleePlayerController.generated.h"

UCLASS()
class POLISHPROJECT_API AMeleePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AMeleePlayerController();

protected:

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category="Input", meta=(ClampMin=0))
	float MouseSensitivity = 1.f;

};
