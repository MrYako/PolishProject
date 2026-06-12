#include "MeleeHero.h"
#include "Components/C_MeleeAttack.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AMeleeHero::AMeleeHero()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bUseControllerRotationYaw = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 5.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

}

void AMeleeHero::OnDeath_Implementation()
{
	Super::OnDeath_Implementation();

	const FVector Current = GetActorLocation();
	const FVector Target(GlideTargetLocation.X, GlideTargetLocation.Y, Current.Z);
	GlideSpeed = FVector::Dist2D(Current, Target) / FMath::Max(GlideDuration, KINDA_SMALL_NUMBER);

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
		PC->DisableInput(PC);

	SetActorTickEnabled(true);
}

void AMeleeHero::ResetCharacter()
{
	Super::ResetCharacter(); // health, collision enabled, MOVE_Walking, meshes visible

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
		PC->EnableInput(PC);

	SetActorTickEnabled(false);
}

void AMeleeHero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetCharacterMovement()->IsFlying()) return;
	UpdateDeathGlide(DeltaTime);
}

void AMeleeHero::UpdateDeathGlide(float DeltaTime)
{
	const FVector Current = GetActorLocation();
	const FVector Target(GlideTargetLocation.X, GlideTargetLocation.Y, Current.Z);
	if (FVector::DistSquared2D(Current, Target) < 1.f) return;

	SetActorLocation(FMath::VInterpConstantTo(Current, Target, DeltaTime, GlideSpeed));
}

void AMeleeHero::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
}

void AMeleeHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMeleeHero::Move);
		EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &AMeleeHero::PerformAttack_Implementation);
	}
}

void AMeleeHero::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero()) return;

	const FRotator CameraYaw(0.f, Camera->GetComponentRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::X);
	const FVector Right   = FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right,   Axis.X);
}
