#include "MeleeAIController.h"
#include "MeleeCharacterBase.h"
#include "MeleeGameState.h"
#include "Components/C_HealthComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"

const FName AMeleeAIController::BBKey_SelfActor(TEXT("SelfActor"));
const FName AMeleeAIController::BBKey_PlayerPawn(TEXT("PlayerPawn"));

void AMeleeAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Set self-reference in Blackboard — lets BTTs access the pawn without a GetControlledPawn cast
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(BBKey_SelfActor, InPawn);
	}

	// Subscribe to GameState's player pawn delegate (push-based — no BT searching each tick)
	if (AMeleeGameState* GS = GetWorld()->GetGameState<AMeleeGameState>())
	{
		GS->OnPlayerPawnSet.AddDynamic(this, &AMeleeAIController::HandlePlayerPawnSet);

		// If player pawn is already available, populate BB immediately
		if (APawn* ExistingPawn = GS->GetPlayerPawn())
		{
			HandlePlayerPawnSet(ExistingPawn);
		}
	}

	// Subscribe to the enemy's death event to stop BT cleanly
	if (AMeleeCharacterBase* MeleeChar = Cast<AMeleeCharacterBase>(InPawn))
	{
		MeleeChar->GetHealthComponent()->OnDeath.AddDynamic(this, &AMeleeAIController::HandlePawnDeath);
	}
}

void AMeleeAIController::OnUnPossess()
{
	// Unsubscribe from GameState to avoid dangling delegates
	if (AMeleeGameState* GS = GetWorld()->GetGameState<AMeleeGameState>())
	{
		GS->OnPlayerPawnSet.RemoveDynamic(this, &AMeleeAIController::HandlePlayerPawnSet);
	}

	Super::OnUnPossess();
}

void AMeleeAIController::HandlePlayerPawnSet(APawn* NewPawn)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(BBKey_PlayerPawn, NewPawn);
	}
}

void AMeleeAIController::HandlePawnDeath()
{
	if (UBrainComponent* Brain = GetBrainComponent())
		Brain->StopLogic(TEXT("Dead"));
}

void AMeleeAIController::Deactivate()
{
	if (UBrainComponent* Brain = GetBrainComponent())
		Brain->StopLogic(TEXT("Inactive"));
}

void AMeleeAIController::Reactivate()
{
	if (UBrainComponent* Brain = GetBrainComponent())
		Brain->RestartLogic();
}
