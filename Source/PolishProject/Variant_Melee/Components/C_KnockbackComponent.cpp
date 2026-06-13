#include "C_KnockbackComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UC_KnockbackComponent::UC_KnockbackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UC_KnockbackComponent::Apply(FVector Direction, int32 ChainDepth)
{
	if (bIsKnockedBack) return;

	CurrentChainDepth = ChainDepth;
	KnockbackActiveDuration = (ChainDepth == 0) ? KnockbackDuration : ChainPushDuration;
	const float DistanceToUse = (ChainDepth == 0) ? KnockbackDistance : ChainPushDistance;

	const FVector FlatDir = FVector(Direction.X, Direction.Y, 0.f).GetSafeNormal();
	if (FlatDir.IsNearlyZero()) return;

	AActor* Owner = GetOwner();
	const FVector Start = Owner->GetActorLocation();
	FVector Target = Start + FlatDir * DistanceToUse;

	FCollisionQueryParams WallTraceParams;
	WallTraceParams.AddIgnoredActor(Owner);

	FHitResult WallHit;
	if (GetWorld()->LineTraceSingleByChannel(WallHit, Start, Target, ECC_WorldStatic, WallTraceParams))
		Target = WallHit.ImpactPoint - FlatDir * 20.f;

	KnockbackStartPos = Start;
	KnockbackTargetPos = Target;
	KnockbackElapsed = 0.f;
	bIsKnockedBack = true;

	if (ACharacter* Char = Cast<ACharacter>(Owner))
		Char->GetCharacterMovement()->DisableMovement();

	SetComponentTickEnabled(true);
	OnKnockbackStarted.Broadcast();
}

void UC_KnockbackComponent::Cancel()
{
	bIsKnockedBack = false;
	GetWorld()->GetTimerManager().ClearTimer(KnockbackRecoveryTimer);
	SetComponentTickEnabled(false);
}

void UC_KnockbackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickKnockback(DeltaTime);
}

void UC_KnockbackComponent::TickKnockback(float DeltaTime)
{
	KnockbackElapsed += DeltaTime;
	const float NormalizedTime = FMath::Clamp(KnockbackElapsed / KnockbackActiveDuration, 0.f, 1.f);
	const float EasedTime = FMath::InterpEaseOut(0.f, 1.f, NormalizedTime, 3.f);

	FHitResult SweepHit;
	GetOwner()->SetActorLocation(FMath::Lerp(KnockbackStartPos, KnockbackTargetPos, EasedTime), /*bSweep=*/true, &SweepHit);

	if (SweepHit.bBlockingHit && CurrentChainDepth < MaxChainDepth)
	{
		if (AActor* OtherActor = SweepHit.GetActor())
		{
			if (UC_KnockbackComponent* OtherKB = OtherActor->FindComponentByClass<UC_KnockbackComponent>())
			{
				if (!OtherKB->IsKnockedBack())
				{
					const FVector PushDir = (OtherActor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
					OtherKB->Apply(PushDir, CurrentChainDepth + 1);
				}
			}
		}
	}

	if (NormalizedTime >= 1.f)
	{
		// Disable tick before starting recovery timer to avoid resetting the timer every frame
		SetComponentTickEnabled(false);
		GetWorld()->GetTimerManager().SetTimer(
			KnockbackRecoveryTimer,
			this, &UC_KnockbackComponent::EndKnockback,
			KnockbackRecoveryTime, /*bLoop=*/false
		);
	}
}

void UC_KnockbackComponent::EndKnockback()
{
	bIsKnockedBack = false;

	if (ACharacter* Char = Cast<ACharacter>(GetOwner()))
		Char->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	OnKnockbackEnded.Broadcast();
}
