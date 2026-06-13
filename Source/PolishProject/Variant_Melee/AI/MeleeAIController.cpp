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

	if (AMeleeGameState* GS = GetWorld()->GetGameState<AMeleeGameState>())
	{
		GS->OnPlayerPawnSet.AddDynamic(this, &AMeleeAIController::HandlePlayerPawnSet);
		if (APawn* ExistingPawn = GS->GetPlayerPawn())
			HandlePlayerPawnSet(ExistingPawn);
	}

	if (AMeleeCharacterBase* MeleeChar = Cast<AMeleeCharacterBase>(InPawn))
	{
		MeleeChar->GetHealthComponent()->OnDeath.AddDynamic(this, &AMeleeAIController::HandlePawnDeath);
		MeleeChar->GetKnockbackComponent()->OnKnockbackStarted.AddDynamic(this, &AMeleeAIController::Deactivate);
		MeleeChar->GetKnockbackComponent()->OnKnockbackEnded.AddDynamic(this, &AMeleeAIController::Reactivate);
	}
}

void AMeleeAIController::OnUnPossess()
{
	// Must unsubscribe before Super clears the pawn reference
	if (AMeleeCharacterBase* MeleeChar = Cast<AMeleeCharacterBase>(GetPawn()))
	{
		MeleeChar->GetKnockbackComponent()->OnKnockbackStarted.RemoveDynamic(this, &AMeleeAIController::Deactivate);
		MeleeChar->GetKnockbackComponent()->OnKnockbackEnded.RemoveDynamic(this, &AMeleeAIController::Reactivate);
	}

	if (AMeleeGameState* GS = GetWorld()->GetGameState<AMeleeGameState>())
		GS->OnPlayerPawnSet.RemoveDynamic(this, &AMeleeAIController::HandlePlayerPawnSet);

	Super::OnUnPossess();
}

void AMeleeAIController::HandlePlayerPawnSet(APawn* NewPawn)
{
	UBrainComponent* Brain = GetBrainComponent();
	if (!(Brain && Brain->IsRunning()) && BehaviorTreeAsset)
		RunBehaviorTree(BehaviorTreeAsset);

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(BBKey_SelfActor, GetPawn());
		BB->SetValueAsObject(BBKey_PlayerPawn, NewPawn);
	}

	SetFocus(NewPawn, EAIFocusPriority::Gameplay);
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
