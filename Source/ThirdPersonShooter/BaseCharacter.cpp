
#include "BaseCharacter.h"
#include "LedgeVolume.h"
#include "ClimbingComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default values for this class
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GamepadRotationRate = 100.0f;
	WalkSpeed = 150.0f;
	StandingSprintSpeed = 450.0f;
	CrouchingSprintSpeed = 300.0f;
	CrouchJumpVelocity = FVector(0.0f, 0.0f, 100.0f);
	CrouchedCapsuleRadius = 50.0f;
	bIsSprinting = false;
	OriginalCapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();

	// Create spring arm and set default values
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	
	// Create camera 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Create climbing component
	ClimbingComponent = CreateDefaultSubobject<UClimbingComponent>(TEXT("Climbing Component"));
	AddOwnedComponent(ClimbingComponent);
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = WalkSpeed;

	// Limit camera's rotation around y-axis
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewPitchMin = -75.0f;
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewPitchMax = 45.0f;
}

void ABaseCharacter::Jump()
{
	Super::Jump();

	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
		LaunchCharacter(CrouchJumpVelocity, false, true);
	}
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Camera control
	PlayerInputComponent->BindAxis(TEXT("LookHorizontal"), this, &ABaseCharacter::AddControllerYawInput);
	PlayerInputComponent->BindAxis(TEXT("LookVertical"), this, &ABaseCharacter::AddControllerPitchInput);

	// Gamepad camera control
	PlayerInputComponent->BindAxis(TEXT("LookHorizontal_Gamepad"), this, &ABaseCharacter::LookHorizontalRate);
	PlayerInputComponent->BindAxis(TEXT("LookVertical_Gamepad"), this, &ABaseCharacter::LookVerticalRate);

	// Movement
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ABaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ABaseCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Pressed, this, &ABaseCharacter::Sprint);
	PlayerInputComponent->BindAction(TEXT("Sprint"), EInputEvent::IE_Released, this, &ABaseCharacter::CancelSprint);
	PlayerInputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Pressed, this, &ABaseCharacter::ActivateCrouch);
	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ABaseCharacter::Jump); 
}

USpringArmComponent* ABaseCharacter::GetSpringArmComponent() const
{
	return SpringArm;
}

UClimbingComponent* ABaseCharacter::GetClimbingComponent() const
{
	return ClimbingComponent;
}

void ABaseCharacter::MoveForward(float Scale)
{
	switch (GetCharacterMovement()->MovementMode)
	{
		case EMovementMode::MOVE_Flying:
			if (Scale > 0.0f)
				ClimbingComponent->ClimbLedge();
			break;
		default:
			FVector ForwardDirection = UKismetMathLibrary::GetForwardVector(FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f));
			AddMovementInput(ForwardDirection, Scale);
			break;
	}
}

void ABaseCharacter::MoveRight(float Scale)
{
	switch (GetCharacterMovement()->MovementMode)
	{
		case EMovementMode::MOVE_Flying:
			LedgeShimmy(Scale);
			break;
		default:
			FVector RightDirection = UKismetMathLibrary::GetRightVector(FRotator(0.0f, Controller->GetControlRotation().Yaw, Controller->GetControlRotation().Roll));
			AddMovementInput(RightDirection, Scale);
			break;
	}
}

void ABaseCharacter::LookHorizontalRate(float Scale)
{
	AddControllerYawInput(Scale * GamepadRotationRate * GetWorld()->GetDeltaSeconds());
}

void ABaseCharacter::LookVerticalRate(float Scale)
{
	AddControllerPitchInput(Scale * GamepadRotationRate * GetWorld()->GetDeltaSeconds());
}

void ABaseCharacter::Sprint()
{
	GetCharacterMovement()->MaxWalkSpeed = StandingSprintSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchingSprintSpeed;
	bIsSprinting = true;
}

void ABaseCharacter::CancelSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = WalkSpeed;
	bIsSprinting = false;
}

void ABaseCharacter::ActivateCrouch()
{
	switch (GetCharacterMovement()->MovementMode)
	{
		case EMovementMode::MOVE_Flying:
			ClimbingComponent->ReleaseLedge();
			CancelSprint();
			break;
		default:
			if (!GetCharacterMovement()->IsCrouching())
			{
				Crouch();
				GetCapsuleComponent()->SetCapsuleRadius(CrouchedCapsuleRadius, true);

				// Perform an evasive roll if entering crouch from sprint
				if (bIsSprinting)
					EvasiveRoll();
			}
			else
			{
				UnCrouch();
				GetCapsuleComponent()->SetCapsuleRadius(OriginalCapsuleRadius, true);
			}
			break;
	}
}

void ABaseCharacter::LedgeShimmy(float Scale)
{
	if (Scale > 0.0f)
	{
		// Shimmy right
		if (ClimbingComponent->SideShimmyTrace(200.0f, 70.0f).bBlockingHit)
			AddMovementInput(GetActorRightVector(), Scale);
		// Turn right forwards
		else if (ClimbingComponent->CornerTrace(TEXT("RightHandSocket"), 50.0f, 25.0f).bBlockingHit)
		{
			ClimbCorner(ClimbingComponent->ClimbRightCornerAnimation, ClimbingComponent->CornerTrace(TEXT("RightHandSocket"), 50.0f, 25.0f).GetActor(), 38.0f);
		}
		// Turn right backwards 
		else if (ClimbingComponent->CornerTrace(TEXT("RightHandSocket"), -50.0f, 25.0f).bBlockingHit)
		{
			ClimbCorner(ClimbingComponent->ClimbRightCornerAnimation, ClimbingComponent->CornerTrace(TEXT("RightHandSocket"), -50.0f, 25.0f).GetActor(), 52.0f);
		}
	}
	else if (Scale < 0.0f)
	{
		// Shimmy left
		if (ClimbingComponent->SideShimmyTrace(200.0f, -70.0f).bBlockingHit)
			AddMovementInput(GetActorRightVector(), Scale);
		// Turn left forwards 
		else if (ClimbingComponent->CornerTrace(TEXT("LeftHandSocket"), 50.0f, -25.0f).bBlockingHit)
		{
			ClimbCorner(ClimbingComponent->ClimbLeftCornerAnimation, ClimbingComponent->CornerTrace(TEXT("LeftHandSocket"), 50.0f, -25.0f).GetActor(), -38.0f);
		}
		// Turn left backwards 
		else if (ClimbingComponent->CornerTrace(TEXT("LeftHandSocket"), -50.0f, -25.0f).bBlockingHit)
		{
			ClimbCorner(ClimbingComponent->ClimbLeftCornerAnimation, ClimbingComponent->CornerTrace(TEXT("LeftHandSocket"), -50.0f, -25.0f).GetActor(), -52.0f);
		}
	}
}
