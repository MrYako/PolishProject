#include "MeleeCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "Components/C_HealthComponent.h"
#include "Components/C_MeleeAttack.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "AIController.h"
#include "BrainComponent.h"

AMeleeCharacterBase::AMeleeCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;  // only active during knockback
	
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AttackComponent = CreateDefaultSubobject<UC_MeleeAttack>(TEXT("AttackComponent"));
	AttackComponent->SetupAttachment(RootComponent);

	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	HitBox->SetupAttachment(AttackComponent);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(AttackComponent);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HealthComponent = CreateDefaultSubobject<UC_HealthComponent>(TEXT("HealthComponent"));

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
}

void AMeleeCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDeath.AddDynamic(this, &AMeleeCharacterBase::HandleDeath);
	HealthComponent->OnDamageReceived.AddDynamic(this, &AMeleeCharacterBase::HandleDamageReceived);

	AttackComponent->SetHitBox(HitBox);

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AMeleeCharacterBase::HandleCapsuleHit);
}

void AMeleeCharacterBase::ReceiveDamage(float Amount, AActor* DamageCauser)
{
	if (GEngine)
	{
		FString CauserName = DamageCauser ? DamageCauser->GetName() : TEXT("None");
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange,
			FString::Printf(TEXT("[%s] Damage %.1f | Causer: %s"), *GetName(), Amount, *CauserName));
	}
	HealthComponent->ApplyDamage(Amount, DamageCauser);
}

void AMeleeCharacterBase::PerformAttack_Implementation()
{
	const bool bCanAttack = AttackComponent->CanAttack();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, bCanAttack ? FColor::Green : FColor::Red,
			FString::Printf(TEXT("[%s] CanAttack: %s"), *GetName(), bCanAttack ? TEXT("YES") : TEXT("NO")));
	}
	if (!bCanAttack) return;

	PlayAttackAnimation();
	AttackComponent->TriggerAttack();
}

void AMeleeCharacterBase::ApplyKnockback(FVector Direction, int32 ChainDepth)
{
    if (bIsKnockedBack) return;   // already reacting — ignore stacked hits

    CurrentKnockbackChainDepth = ChainDepth;
    KnockbackActiveDuration = (ChainDepth == 0) ? KnockbackDuration : ChainPushDuration;
    const float DistanceToUse = (ChainDepth == 0) ? KnockbackDistance : ChainPushDistance;

    // Project direction onto horizontal plane and normalise
    const FVector FlatDir = FVector(Direction.X, Direction.Y, 0.f).GetSafeNormal();
    if (FlatDir.IsNearlyZero()) return;

    // Compute target — clamp to wall if blocked
    const FVector Start = GetActorLocation();
    FVector Target = Start + FlatDir * DistanceToUse;

    FHitResult WallHit;
    if (GetWorld()->LineTraceSingleByChannel(WallHit, Start, Target, ECC_WorldStatic))
    {
        // Stop 20 cm before the wall surface
        Target = WallHit.ImpactPoint - FlatDir * 20.f;
    }

    KnockbackStartPos = Start;
    KnockbackTargetPos = Target;
    KnockbackElapsed = 0.f;
    bIsKnockedBack = true;

    // Pause AI logic for the duration
    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (UBrainComponent* Brain = AIC->GetBrainComponent())
        {
            Brain->StopLogic(TEXT("Knockback"));
        }
    }

    SetActorTickEnabled(true);
}

void AMeleeCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsKnockedBack) return;

    KnockbackElapsed += DeltaTime;
    const float Alpha = FMath::Clamp(KnockbackElapsed / KnockbackActiveDuration, 0.f, 1.f);
    // Ease-out: fast start, decelerates to target (exponent 3)
    const float EasedAlpha = FMath::InterpEaseOut(0.f, 1.f, Alpha, 3.f);

    const FVector NewPos = FMath::Lerp(KnockbackStartPos, KnockbackTargetPos, EasedAlpha);
    SetActorLocation(NewPos, /*bSweep=*/true);

    if (Alpha >= 1.f)
    {
        bIsKnockedBack = false;
        GetWorld()->GetTimerManager().SetTimer(
            KnockbackRecoveryTimer,
            this, &AMeleeCharacterBase::EndKnockback,
            KnockbackRecoveryTime, /*bLoop=*/false
        );
    }
}

void AMeleeCharacterBase::EndKnockback()
{
    SetActorTickEnabled(false);

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (UBrainComponent* Brain = AIC->GetBrainComponent())
        {
            Brain->RestartLogic();
        }
    }
}

void AMeleeCharacterBase::HandleCapsuleHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!bIsKnockedBack) return;
    if (CurrentKnockbackChainDepth >= MaxKnockbackChainDepth) return;

    AMeleeCharacterBase* OtherChar = Cast<AMeleeCharacterBase>(OtherActor);
    if (!OtherChar || OtherChar->bIsKnockedBack) return;

    const FVector PushDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    OtherChar->ApplyKnockback(PushDir, CurrentKnockbackChainDepth + 1);
}

void AMeleeCharacterBase::OnDeath_Implementation()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	BodyMesh->SetHiddenInGame(true);
	WeaponMesh->SetHiddenInGame(true);
}

void AMeleeCharacterBase::Disable() const
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	BodyMesh->SetHiddenInGame(true);
	WeaponMesh->SetHiddenInGame(true);
}

void AMeleeCharacterBase::ResetCharacter()
{
	HealthComponent->Reset();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	bIsKnockedBack = false;
	GetWorld()->GetTimerManager().ClearTimer(KnockbackRecoveryTimer);
	SetActorTickEnabled(false);
	BodyMesh->SetHiddenInGame(false);
	WeaponMesh->SetHiddenInGame(false);
}

void AMeleeCharacterBase::HandleDeath()
{
	OnDeath();
	OnCharacterDied.Broadcast(this);
}

void AMeleeCharacterBase::HandleDamageReceived(float Damage, AActor* DamageInstigator)
{
	OnDamageReceived(Damage, DamageInstigator);
}

void AMeleeCharacterBase::PostInitProperties()
{
	Super::PostInitProperties();
	HitBox->SetCollisionProfileName(TEXT("Trigger"));
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
