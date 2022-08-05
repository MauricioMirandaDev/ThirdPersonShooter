
#include "BaseCharacter.h"
#include "LedgeVolume.h"
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
	ClimbLedgeAnimation = nullptr;
	ClimbRightCornerAnimation = nullptr;
	ClimbLeftCornerAnimation = nullptr;
	bIsDroppingToLedge = false;

	// Create spring arm and set default values
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	
	// Create camera 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
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

	OriginalYawMin = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMin;
	OriginalYawMax = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMax;
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

void ABaseCharacter::MoveForward(float Scale)
{
	if (!GetCharacterMovement()->IsFlying())
	{
		FVector ForwardDirection = UKismetMathLibrary::GetForwardVector(FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f));
		AddMovementInput(ForwardDirection, Scale);
	}
	else if (Scale > 0.0f)
	{
		ClimbLedge();
	}
}

void ABaseCharacter::MoveRight(float Scale)
{
	if (GetCharacterMovement()->IsFlying())
	{
		if (Scale > 0.0f)
		{
			// Shimmy right
			if (CanSideShimmy(200.0f, 70.0f))
				AddMovementInput(GetActorRightVector(), Scale); 
			// Turn right forwards
			else if (CanCornerShimmy(TEXT("RightHandSocket"), 50.0f, 25.0f).bBlockingHit)
			{
				ClimbCorner(ClimbRightCornerAnimation, CanCornerShimmy(TEXT("RightHandSocket"), 50.0f, 25.0f).GetActor(), 38.0f); 
			}
			// Turn right backwards
			else if (CanCornerShimmy(TEXT("RightHandSocket"), -50.0f, 25.0f).bBlockingHit)
			{
				ClimbCorner(ClimbRightCornerAnimation, CanCornerShimmy(TEXT("RightHandSocket"), -50.0f, 25.0f).GetActor(), 52.0f); 
			}
		}
		else if (Scale < 0.0f)
		{
			// Shimmy left
			if (CanSideShimmy(200.0f, -70.0f))
				AddMovementInput(GetActorRightVector(), Scale); 
			// Turn left forwards
			else if (CanCornerShimmy(TEXT("LeftHandSocket"), 50.0f, -25.0f).bBlockingHit)
			{
				ClimbCorner(ClimbLeftCornerAnimation, CanCornerShimmy(TEXT("LeftHandSocket"), 50.0f, -25.0f).GetActor(), -38.0f);
			}
			// Turn left backwards
			else if (CanCornerShimmy(TEXT("LeftHandSocket"), -50.0f, -25.0f).bBlockingHit)
			{
				ClimbCorner(ClimbLeftCornerAnimation, CanCornerShimmy(TEXT("LeftHandSocket"), -50.0f, -25.0f).GetActor(), -52.0f);
			}
		}
	}
	else
	{
		FVector RightDirection = UKismetMathLibrary::GetRightVector(FRotator(0.0f, Controller->GetControlRotation().Yaw, Controller->GetControlRotation().Roll));
		AddMovementInput(RightDirection, Scale);
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
			ReleaseLedge();
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

FHitResult ABaseCharacter::ForwardTrace(float Range)
{
	FHitResult ForwardHitResult;
	FCollisionQueryParams ForwardQueryParams;
	ForwardQueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OUT ForwardHitResult, GetActorLocation(), GetActorLocation() + (GetActorForwardVector() * Range), 
										 ECollisionChannel::ECC_Visibility, ForwardQueryParams);

	return ForwardHitResult;
}

FHitResult ABaseCharacter::ClimbUpTrace(float TraceHeight, float TraceLength, float SphereRadius)
{
	FHitResult ClimbHitResult;
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), GetActorLocation() + (GetActorUpVector() * TraceHeight), GetActorLocation() + (GetActorForwardVector() * TraceLength),
					 SphereRadius, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), true, IgnoredActors, EDrawDebugTrace::None, OUT ClimbHitResult, true);

	return ClimbHitResult;
}

bool ABaseCharacter::CanSideShimmy(float UpRange, float SideRange)
{
	FHitResult SideHitResult;

	FCollisionQueryParams SideQueryParams;
	SideQueryParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel1);

	return GetWorld()->LineTraceSingleByObjectType(SideHitResult, GetActorLocation(), ((GetActorUpVector() * UpRange) + (GetActorRightVector() * SideRange)) + GetActorLocation(), 
												   ObjectQueryParams, SideQueryParams);;
}

FHitResult ABaseCharacter::CanCornerShimmy(FName SocketName, float ForwardRange, float SideRange)
{
	FHitResult CornerHitResult;

	FCollisionQueryParams CornerQueryParams;
	CornerQueryParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel1);

	FVector Start = GetMesh()->GetSocketLocation(SocketName) + (GetActorForwardVector() * ForwardRange);
	FVector End = Start + (GetActorRightVector() * SideRange);
	GetWorld()->LineTraceSingleByObjectType(OUT CornerHitResult, Start, End, ObjectQueryParams, CornerQueryParams);

	return CornerHitResult;
}

FHitResult ABaseCharacter::CanDropToLedge()
{
	FHitResult DropHitResult;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));

	FVector Start = GetActorLocation() + (GetActorUpVector() * -1.0f * (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 25.0f));
	FVector End = Start + (GetActorForwardVector() * -100.0f);

	UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), Start, End, 30.0f, ObjectTypes, true, ActorsToIgnore, EDrawDebugTrace::None, OUT DropHitResult, true);

	return DropHitResult;
}

void ABaseCharacter::LedgeGrab(AActor* CurrentLedge, FVector TargetLocation, bool bIsLedgeDrop)
{
	// Set variables
	DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpringArm->bDoCollisionTest = false;

	// Information for ledge grab
	FVector GrabLocation = TargetLocation - (GetActorForwardVector() * 5.0f);
	FLatentActionInfo LatentActionInfo;
	LatentActionInfo.CallbackTarget = this;
	FRotator GrabRotation = (CurrentLedge->GetActorForwardVector() * -1.0f).Rotation();

	// Limit camera rotation
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMin = GrabRotation.Yaw - 45.0f;
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMax = GrabRotation.Yaw + 45.0f;

	// Move player to grab ledge
	if (bIsLedgeDrop)
	{
		GrabLocation = TargetLocation + (GetActorForwardVector() * 5.0f);

		PlayAnimMontage(ClimbLeftCornerAnimation, 1.0f, TEXT("None"));

		UKismetSystemLibrary::MoveComponentTo(RootComponent, FVector(GrabLocation.X, GrabLocation.Y, CurrentLedge->GetActorLocation().Z - 115.0f),
			GrabRotation, true, true, ClimbLeftCornerAnimation->SequenceLength, true, EMoveComponentAction::Move, LatentActionInfo);
	}
	else
	{
		UKismetSystemLibrary::MoveComponentTo(RootComponent, FVector(GrabLocation.X, GrabLocation.Y, CurrentLedge->GetActorLocation().Z - 115.0f),
			GrabRotation, true, true, 0.1f, true, EMoveComponentAction::Move, LatentActionInfo);
	}

	// Add a delay to stop ignoring input
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
	{
		EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	}, 1.0f, false);
}

void ABaseCharacter::ReleaseLedge()
{
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMin = OriginalYawMin;
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMax = OriginalYawMax;

	SpringArm->bDoCollisionTest = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	UnCrouch();
	CancelSprint();
}

void ABaseCharacter::ClimbLedge()
{
	if (FVector::DotProduct(ClimbUpTrace(200.0f, 150.0f, 30.0f).ImpactNormal, GetActorUpVector()) == 1.0f && ClimbLedgeAnimation)
		PlayAnimMontage(ClimbLedgeAnimation, 1.0f, TEXT("None"));
}

void ABaseCharacter::DropToLedge()
{
	if (GetVelocity().Length() <= WalkSpeed && CanDropToLedge().bBlockingHit)
	{
		if (FVector::DotProduct(CanDropToLedge().GetActor()->GetActorForwardVector(), GetActorForwardVector()))
			LedgeGrab(CanDropToLedge().GetActor(), CanDropToLedge().ImpactPoint, true);
	}
}
