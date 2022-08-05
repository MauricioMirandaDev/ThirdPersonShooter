
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;

UCLASS()
class THIRDPERSONSHOOTER_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent)
	void EvasiveRoll();

	UFUNCTION(BlueprintImplementableEvent)
	void ClimbCorner(UAnimMontage* CornerAnimMontage, AActor* TargetLedge, float Multiplier);

	UPROPERTY(EditAnywhere, BlueprintReadWrite);
	bool bIsDroppingToLedge;

	FHitResult ForwardTrace(float Range);

	UFUNCTION(BlueprintCallable)
	void LedgeGrab(AActor* CurrentLedge, FVector TargetLocation, bool bIsLedgeDrop);

	void ReleaseLedge();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Jump() override; 

private:
	// Components for camera control
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	float OriginalYawMin;

	float OriginalYawMax; 

	// How fast to rotate camera on gamepad
	UPROPERTY(EditAnywhere, Category = "Input")
	float GamepadRotationRate;

	// Speed when walking and crouch walking
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed;

	// Speed when sprinting while standing
	UPROPERTY(EditAnywhere, Category = "Movement")
	float StandingSprintSpeed;

	// Speed when sprinting while crouching
	UPROPERTY(EditAnywhere, Category = "Movement")
	float CrouchingSprintSpeed;

	// How high the player jumps from crouch
	UPROPERTY(EditAnywhere, Category = "Movement")
	FVector CrouchJumpVelocity;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CrouchedCapsuleRadius; 

	// Functions and variables for movement 
	void MoveForward(float Scale);

	void MoveRight(float Scale);

	void LookHorizontalRate(float Scale);

	void LookVerticalRate(float Scale);

	void Sprint(); 

	void CancelSprint();

	void ActivateCrouch(); 

	bool bIsSprinting;

	float OriginalCapsuleRadius;

	// Functions for ledge grab and ledge shimmy
	UPROPERTY(EditAnywhere, Category = "Climbing")
	UAnimMontage* ClimbLedgeAnimation;

	UPROPERTY(EditAnywhere, Category = "Climbing")
	UAnimMontage* ClimbRightCornerAnimation;

	UPROPERTY(EditAnywhere, Category = "Climbing")
	UAnimMontage* ClimbLeftCornerAnimation;

	UFUNCTION(BlueprintCallable)
	void DropToLedge();

	FHitResult ClimbUpTrace(float TraceHeight, float TraceLength, float SphereRadius);

	bool CanSideShimmy(float UpRange, float SideRange);

	FHitResult CanCornerShimmy(FName SocketName, float ForwardRange, float SideRange);

	FHitResult CanDropToLedge();

	void ClimbLedge();
};
