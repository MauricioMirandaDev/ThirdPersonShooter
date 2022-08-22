
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbingComponent.generated.h"

class UAnimMontage;
class ABaseCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THIRDPERSONSHOOTER_API UClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UClimbingComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Animations for climbing corners
	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* ClimbRightCornerAnimation;

	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* ClimbLeftCornerAnimation;

	// Traces
	FHitResult ForwardTrace(float ForwardRange);

	FHitResult ClimbUpTrace(float UpRange, float ForwardRange);

	FHitResult DropDownTrace();

	FHitResult SideShimmyTrace(float UpRange, float SideRange);

	FHitResult CornerTrace(FName SocketName, float ForwardRange, float SideRange);

	// Ledge grabbing functions
	void GrabLedge(FVector TargetLocation, AActor* CurrentLedge, bool bIsLedgeDrop);

	void ReleaseLedge();

	void ClimbLedge();

	UFUNCTION(BlueprintCallable)
	void DropToLedge();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// Animations used for climbing ledges
	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* ClimbLedgeAnimation;
		
	UPROPERTY(EditAnywhere, Category = "Animations")
	UAnimMontage* DropToLedgeAnimation;

	// Max speed the player can be moving in order to grab ledge from drop
	UPROPERTY(EditAnywhere, Category = "Data")
	float MaxWalkOffSpeed;

	// Information for traces / ledge grabbing
	ABaseCharacter* OwnerCharacter;

	TArray<AActor*> ActorsToIgnore;

	TArray<TEnumAsByte<EObjectTypeQuery>> Ledges;

	float OriginalYawMin, OriginalYawMax;
};
