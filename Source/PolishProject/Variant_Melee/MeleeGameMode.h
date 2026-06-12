#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MeleeGameState.h"
#include "MeleeGameMode.generated.h"

class AMeleeCharacterBase;
class AMeleePlayerState;

UCLASS()
class POLISHPROJECT_API AMeleeGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	AMeleeGameMode();

protected:

	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	/** Enemy class to pre-spawn for the pool. Set in Blueprint. */
	UPROPERTY(EditDefaultsOnly, Category="Wave|Pool")
	TSubclassOf<AMeleeCharacterBase> EnemyClass;

	/** World transform where dead enemies wait (should be off-screen). */
	UPROPERTY(EditDefaultsOnly, Category="Wave|Pool")
	FTransform HoldingAreaTransform;

	/** Total enemies to pre-spawn at game start. Should be >= max EnemyCount across all waves. */
	UPROPERTY(EditDefaultsOnly, Category="Wave|Pool", meta=(ClampMin=1))
	int32 InitialPoolSize = 10;

	/** Minimum distance from player for an out-of-view spawn point. */
	UPROPERTY(EditDefaultsOnly, Category="Wave|Spawn", meta=(ClampMin=100, Units="cm"))
	float MinSpawnDistance = 1500.f;

private:

	TArray<TObjectPtr<AMeleeCharacterBase>> AvailablePool;
	TArray<TObjectPtr<AMeleeCharacterBase>> ActiveEnemies;

	FTimerHandle WaveStartTimer;
	FTimerHandle RespawnTimer;

	void PrewarmPool();
	void StartWave();
	void SpawnGroup(const FEnemyGroupConfig& Group, const FVector& PlayerLocation);
	void CheckWaveComplete();
	void ResetGame();

	UFUNCTION()
	void OnEnemyKilled(AMeleeCharacterBase* Enemy);

	UFUNCTION()
	void OnHeroDied(AMeleeCharacterBase* Character);
};
