
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;
class UClimbingComponent;

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

	// Functions to gain access to components
	USpringArmComponent* GetSpringArmComponent() const;

	UClimbingComponent* GetClimbingComponent() const;

	UFUNCTION(BlueprintImplementableEvent)
	void EvasiveRoll();

	UFUNCTION(BlueprintImplementableEvent)
	void ClimbCorner(UAnimMontage* CornerAnimMontage, AActor* TargetLedge, float Multiplier);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Jump() override; 

private:
	// Components and variables for camera control
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Input")
	float GamepadRotationRate;

	// Components, functions, and variables for movement
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float StandingSprintSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CrouchingSprintSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	FVector CrouchJumpVelocity;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float CrouchedCapsuleRadius; 

	void MoveForward(float Scale);

	void MoveRight(float Scale);

	void LookHorizontalRate(float Scale);

	void LookVerticalRate(float Scale);

	void Sprint(); 

	void CancelSprint();

	void ActivateCrouch(); 

	bool bIsSprinting;

	float OriginalCapsuleRadius;

	// Component and function for ledge grabbing and climbing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UClimbingComponent* ClimbingComponent;

	void LedgeShimmy(float Scale);
};
