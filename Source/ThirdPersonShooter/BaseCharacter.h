
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// Components for camera control
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

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

	// Functions for movement 
	void MoveForward(float Scale);

	void MoveRight(float Scale);

	void LookHorizontalRate(float Scale);

	void LookVerticalRate(float Scale);

	void Sprint(); 

	void CancelSprint();

	void ActivateCrouch(); 
};
