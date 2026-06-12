#include "MeleeCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "Components/C_HealthComponent.h"
#include "Components/C_MeleeAttack.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

AMeleeCharacterBase::AMeleeCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
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

void AMeleeCharacterBase::ApplyKnockback(FVector Direction, float Strength)
{
	LaunchCharacter(Direction.GetSafeNormal() * Strength, true, false);
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
