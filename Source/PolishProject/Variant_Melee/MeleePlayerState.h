#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MeleePlayerState.generated.h"

UCLASS()
class POLISHPROJECT_API AMeleePlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	void IncrementKillCount();
	void ResetStats();

	UFUNCTION(BlueprintPure, Category="Stats")
	int32 GetKillCount() const { return KillCount; }

private:

	UPROPERTY(BlueprintReadOnly, Category="Stats", meta=(AllowPrivateAccess=true))
	int32 KillCount = 0;
};
