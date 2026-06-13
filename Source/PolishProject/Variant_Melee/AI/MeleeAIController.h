#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "MeleeAIController.generated.h"

class AMeleeCharacterBase;

UCLASS()
class POLISHPROJECT_API AMeleeAIController : public AAIController
{
	GENERATED_BODY()

public:

	static const FName BBKey_SelfActor;
	static const FName BBKey_PlayerPawn;

	UFUNCTION()
	void Deactivate();

	UFUNCTION()
	void Reactivate();

	UPROPERTY(EditDefaultsOnly, Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

protected:

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:

	UFUNCTION()
	void HandlePlayerPawnSet(APawn* NewPawn);

	UFUNCTION()
	void HandlePawnDeath();
};
