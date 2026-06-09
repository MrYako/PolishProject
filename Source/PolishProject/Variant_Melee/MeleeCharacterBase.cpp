#include "MeleeCharacterBase.h"
#include "DrawDebugHelpers.h"
#include "Components/C_HealthComponent.h"
#include "Components/C_MeleeAttack.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	HealthComponent->ApplyDamage(Amount, DamageCauser);
}

void AMeleeCharacterBase::PerformAttack()
{
	if (!AttackComponent->CanAttack()) return;
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

void AMeleeCharacterBase::HandleDeath()
{
	OnDeath();
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
