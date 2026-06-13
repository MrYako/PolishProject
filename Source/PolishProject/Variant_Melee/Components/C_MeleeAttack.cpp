#include "C_MeleeAttack.h"
#include "Components/BoxComponent.h"
#include "Variant_Melee/MeleeCharacterBase.h"

UC_MeleeAttack::UC_MeleeAttack()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UC_MeleeAttack::BeginPlay()
{
	Super::BeginPlay();
}

void UC_MeleeAttack::SetHitBox(UBoxComponent* Box)
{
	HitBox = Box;
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &UC_MeleeAttack::OnHitBoxOverlap);
}

void UC_MeleeAttack::TriggerAttack()
{
	if (!CanAttack()) return;
	
	bCanAttack = false;
	HitActorsThisSwing.Empty();
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &UC_MeleeAttack::EndAttack, HitBoxActiveDuration, false);
}

void UC_MeleeAttack::EndAttack()
{
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorld()->GetTimerManager().SetTimer(CooldownTimer, [this]()
	{
		bCanAttack = true;
	}, AttackCooldown - HitBoxActiveDuration, false);
}

void UC_MeleeAttack::OnHitBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	AMeleeCharacterBase* Target = Cast<AMeleeCharacterBase>(OtherActor);
	if (!Target || Target == GetOwner()) return;
	if (HitActorsThisSwing.Contains(OtherActor)) return;

	HitActorsThisSwing.Add(OtherActor);

	const FVector OwnerForward = GetOwner()->GetActorForwardVector();
	const FVector SweepRight = FVector::CrossProduct(OwnerForward, FVector::UpVector).GetSafeNormal();

	const FVector AwayDir = (OtherActor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
	const float SideSign = FVector::DotProduct(SweepRight, AwayDir);

	FVector KnockbackDir;
	if (FMath::Abs(SideSign) < 0.1f)
	{
		// Enemy is directly in front — no lateral fan component
		KnockbackDir = AwayDir;
	}
	else
	{
		const FVector FanComponent = SweepRight * FMath::Sign(SideSign);
		KnockbackDir = FMath::Lerp(AwayDir, FanComponent, KnockbackFanBlend).GetSafeNormal();
	}

	Target->ReceiveDamage(AttackDamage, GetOwner());
	Target->ApplyKnockback(KnockbackDir);

	OnHit.Broadcast(OtherActor);
}
