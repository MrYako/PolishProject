#include "MeleeGameMode.h"
#include "MeleeCharacterBase.h"
#include "MeleeGameState.h"
#include "MeleePlayerState.h"
#include "AI/MeleeAIController.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

AMeleeGameMode::AMeleeGameMode()
{
	GameStateClass = AMeleeGameState::StaticClass();
	PlayerStateClass = AMeleePlayerState::StaticClass();
}

void AMeleeGameMode::BeginPlay()
{
	Super::BeginPlay();
	PrewarmPool();
}

void AMeleeGameMode::PrewarmPool()
{
	if (!EnemyClass) return;

	AvailablePool.Reserve(InitialPoolSize);
	ActiveEnemies.Reserve(InitialPoolSize);

	for (int32 i = 0; i < InitialPoolSize; ++i)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AMeleeCharacterBase* Enemy = GetWorld()->SpawnActor<AMeleeCharacterBase>(
			EnemyClass, HoldingAreaTransform, Params);

		if (Enemy)
		{
			Enemy->Disable();
			Enemy->OnCharacterDied.AddDynamic(this, &AMeleeGameMode::OnEnemyKilled);
			AvailablePool.Add(Enemy);
		}
	}

	// Start the first wave after the initial delay
	AMeleeGameState* GS = GetGameState<AMeleeGameState>();
	const float Delay = GS ? GS->GetCurrentWaveConfig().StartDelay : 3.f;
	GetWorldTimerManager().SetTimer(WaveStartTimer, this, &AMeleeGameMode::StartWave, Delay, false);
}

void AMeleeGameMode::StartWave()
{
	AMeleeGameState* GS = GetGameState<AMeleeGameState>();
	if (!GS) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	const FVector PlayerLocation = PC && PC->GetPawn()
		? PC->GetPawn()->GetActorLocation()
		: HoldingAreaTransform.GetLocation();

	for (const FEnemyGroupConfig& Group : GS->GetCurrentWaveConfig().Groups)
	{
		SpawnGroup(Group, PlayerLocation);
	}
}

void AMeleeGameMode::SpawnGroup(const FEnemyGroupConfig& Group, const FVector& PlayerLocation)
{
	const int32 SpawnCount = FMath::Min(Group.EnemyCount, AvailablePool.Num());
	if (SpawnCount == 0) return;

	const float AngleDeg = Group.bRandomDirection ? FMath::FRandRange(0.f, 360.f) : Group.SpawnDirectionDeg;
	const float AngleRad = FMath::DegreesToRadians(AngleDeg);
	const FVector GroupCenter = PlayerLocation + FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f) * MinSpawnDistance;

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		AMeleeCharacterBase* Enemy = AvailablePool.Last();
		AvailablePool.RemoveAt(AvailablePool.Num() - 1);

		const float RandAngle = FMath::FRandRange(0.f, 2.f * PI);
		const float RandRadius = FMath::Sqrt(FMath::FRandRange(0.f, 1.f)) * Group.SpawnRadius;
		const FVector SpawnLocation = GroupCenter + FVector(FMath::Cos(RandAngle) * RandRadius,
		                                                    FMath::Sin(RandAngle) * RandRadius, 0.f);
		Enemy->SetActorLocation(SpawnLocation);
		Enemy->ResetCharacter();
		if (AMeleeAIController* AIC = Enemy->GetController<AMeleeAIController>())
			AIC->Reactivate();
		ActiveEnemies.Add(Enemy);
	}
}

void AMeleeGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);

	APlayerController* PC = Cast<APlayerController>(NewPlayer);
	if (!PC) return;

	if (AMeleeGameState* GS = GetGameState<AMeleeGameState>())
		GS->SetPlayerPawn(PC->GetPawn());

	if (AMeleeCharacterBase* Hero = Cast<AMeleeCharacterBase>(PC->GetPawn()))
		Hero->OnCharacterDied.AddDynamic(this, &AMeleeGameMode::OnHeroDied);
}

void AMeleeGameMode::OnEnemyKilled(AMeleeCharacterBase* Enemy)
{
	if (!ActiveEnemies.Contains(Enemy)) return;

	// Award kill to the local player's PlayerState
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AMeleePlayerState* PS = PC->GetPlayerState<AMeleePlayerState>())
		{
			PS->IncrementKillCount();
		}
	}

	// Return to holding area (actor is already visually dead via OnDeath_Implementation)
	Enemy->SetActorLocation(HoldingAreaTransform.GetLocation());
	ActiveEnemies.Remove(Enemy);
	AvailablePool.Add(Enemy);

	CheckWaveComplete();
}

void AMeleeGameMode::CheckWaveComplete()
{
	if (!ActiveEnemies.IsEmpty()) return;

	AMeleeGameState* GS = GetGameState<AMeleeGameState>();
	if (!GS) return;

	GS->AdvanceWave();

	const float Delay = GS->GetCurrentWaveConfig().StartDelay;
	GetWorldTimerManager().SetTimer(WaveStartTimer, this, &AMeleeGameMode::StartWave, Delay, false);
}

void AMeleeGameMode::ResetGame()
{
	GetWorldTimerManager().ClearTimer(WaveStartTimer);
	GetWorldTimerManager().ClearTimer(RespawnTimer);

	// Force all living enemies back into the pool
	for (AMeleeCharacterBase* Enemy : ActiveEnemies)
	{
		Enemy->SetActorLocation(HoldingAreaTransform.GetLocation());
		Enemy->Disable();
		if (AMeleeAIController* AIC = Enemy->GetController<AMeleeAIController>())
			AIC->Deactivate();
		AvailablePool.Add(Enemy);
	}
	ActiveEnemies.Reset();

	if (AMeleeGameState* GS = GetGameState<AMeleeGameState>())
		GS->ResetWaves();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		if (AMeleePlayerState* PS = PC->GetPlayerState<AMeleePlayerState>())
			PS->ResetStats();
}

void AMeleeGameMode::OnHeroDied(AMeleeCharacterBase* /*Character*/)
{
	ResetGame();

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// Hero glides to center; after 3 seconds restore state in-place (same pawn, no respawn)
	GetWorldTimerManager().SetTimer(RespawnTimer, [this, PC]()
	{
		if (AMeleeCharacterBase* Hero = Cast<AMeleeCharacterBase>(PC->GetPawn()))
			Hero->ResetCharacter();

		if (AMeleeGameState* GS = GetGameState<AMeleeGameState>())
		{
			const float Delay = GS->GetCurrentWaveConfig().StartDelay;
			GetWorldTimerManager().SetTimer(WaveStartTimer, this, &AMeleeGameMode::StartWave, Delay, false);
		}
	}, 3.f, false);
}
