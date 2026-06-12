#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MeleeGameState.generated.h"

USTRUCT(BlueprintType)
struct FEnemyGroupConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Group", meta=(ClampMin=1))
	int32 EnemyCount = 3;

	/** Pick a random approach angle instead of SpawnDirectionDeg. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Group")
	bool bRandomDirection = false;

	/** World-space angle (0=+X, 90=+Y). Ignored when bRandomDirection is true. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Group",
		meta=(ClampMin=0, ClampMax=360, Units="deg", EditCondition="!bRandomDirection"))
	float SpawnDirectionDeg = 0.f;

	/** Radius of the scatter area around the group center. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Group", meta=(ClampMin=0, Units="cm"))
	float SpawnRadius = 300.f;
};

USTRUCT(BlueprintType)
struct FMeleeWaveConfig
{
	GENERATED_BODY()

	/** Delay before this wave starts after the previous one ends. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Wave", meta=(ClampMin=0, Units="s"))
	float StartDelay = 3.f;

	/** Enemy groups that spawn simultaneously at wave start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Wave")
	TArray<FEnemyGroupConfig> Groups;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerPawnSet, APawn*, NewPawn);

UCLASS()
class POLISHPROJECT_API AMeleeGameState : public AGameState
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Waves")
	TArray<FMeleeWaveConfig> Waves;

	UPROPERTY(BlueprintReadOnly, Category="Waves")
	int32 CurrentWaveIndex = 0;

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnPlayerPawnSet OnPlayerPawnSet;

	void SetPlayerPawn(APawn* Pawn);

	UFUNCTION(BlueprintPure, Category="Game")
	APawn* GetPlayerPawn() const { return CachedPlayerPawn.Get(); }

	FMeleeWaveConfig GetCurrentWaveConfig() const;
	void AdvanceWave();
	void ResetWaves();

private:

	TObjectPtr<APawn> CachedPlayerPawn;
};
