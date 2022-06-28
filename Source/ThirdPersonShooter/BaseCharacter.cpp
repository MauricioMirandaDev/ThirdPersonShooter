
#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
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
}

void ABaseCharacter::MoveForward(float Scale)
{
	FVector ForwardDirection = UKismetMathLibrary::GetForwardVector(FRotator(0.0f, Controller->GetControlRotation().Yaw, 0.0f));
	AddMovementInput(ForwardDirection, Scale);
}

void ABaseCharacter::MoveRight(float Scale)
{
	FVector RightDirection = UKismetMathLibrary::GetRightVector(FRotator(0.0f, Controller->GetControlRotation().Yaw, Controller->GetControlRotation().Roll));
	AddMovementInput(RightDirection, Scale); 
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
}

void ABaseCharacter::CancelSprint()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = WalkSpeed;
}

void ABaseCharacter::ActivateCrouch()
{
	if (!GetCharacterMovement()->IsCrouching())
		Crouch();
	else
		UnCrouch();
}
