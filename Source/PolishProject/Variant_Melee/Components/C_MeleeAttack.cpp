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

	const FVector KnockbackDir = (OtherActor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();

	Target->ReceiveDamage(AttackDamage, GetOwner());
	Target->ApplyKnockback(KnockbackDir, KnockbackStrength);

	OnHit.Broadcast(OtherActor);
}
