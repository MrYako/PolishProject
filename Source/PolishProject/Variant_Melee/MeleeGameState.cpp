#include "MeleeGameState.h"

void AMeleeGameState::SetPlayerPawn(APawn* Pawn)
{
	CachedPlayerPawn = Pawn;
	OnPlayerPawnSet.Broadcast(Pawn);
}

FMeleeWaveConfig AMeleeGameState::GetCurrentWaveConfig() const
{
	if (Waves.IsValidIndex(CurrentWaveIndex))
	{
		return Waves[CurrentWaveIndex];
	}
	// Fallback: repeat last wave if index exceeds the array
	if (Waves.Num() > 0)
	{
		return Waves.Last();
	}
	return FMeleeWaveConfig{};
}

void AMeleeGameState::AdvanceWave()
{
	if (CurrentWaveIndex < Waves.Num() - 1)
	{
		++CurrentWaveIndex;
	}
}

void AMeleeGameState::ResetWaves()
{
	CurrentWaveIndex = 0;
}
