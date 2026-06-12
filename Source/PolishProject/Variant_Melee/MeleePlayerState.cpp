#include "MeleePlayerState.h"

void AMeleePlayerState::IncrementKillCount()
{
	++KillCount;
}

void AMeleePlayerState::ResetStats()
{
	KillCount = 0;
}
