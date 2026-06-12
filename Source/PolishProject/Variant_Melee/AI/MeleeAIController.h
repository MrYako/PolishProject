#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MeleeAIController.generated.h"

class AMeleeCharacterBase;

UCLASS()
class POLISHPROJECT_API AMeleeAIController : public AAIController
{
	GENERATED_BODY()

public:

	static const FName BBKey_SelfActor;
	static const FName BBKey_PlayerPawn;

	void Deactivate();
	void Reactivate();

protected:

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:

	UFUNCTION()
	void HandlePlayerPawnSet(APawn* NewPawn);

	UFUNCTION()
	void HandlePawnDeath();
};
