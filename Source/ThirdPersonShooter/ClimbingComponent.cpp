
#include "ClimbingComponent.h"
#include "BaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UClimbingComponent::UClimbingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxWalkOffSpeed = 100.0f;
}

// Called when the game starts
void UClimbingComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ABaseCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!OwnerCharacter)
		return;

	ActorsToIgnore.Add(OwnerCharacter);
	Ledges.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_GameTraceChannel1));

	OriginalYawMin = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMin;
	OriginalYawMax = UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMax;
}

// Called every frame
void UClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// Get hit result of surface in front of player
FHitResult UClimbingComponent::ForwardTrace(float ForwardRange)
{
	FHitResult ForwardTraceResult;

	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), OwnerCharacter->GetActorLocation(), 
											OwnerCharacter->GetActorLocation() + (OwnerCharacter->GetActorForwardVector() * ForwardRange),
											30.0f, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), true, ActorsToIgnore, EDrawDebugTrace::None,
											OUT ForwardTraceResult, true);

	return ForwardTraceResult;
}

// Get hit result of surface above the player
FHitResult UClimbingComponent::ClimbUpTrace(float UpRange, float ForwardRange)
{
	FHitResult ClimbUpTraceResult;

	UKismetSystemLibrary::SphereTraceSingle(GetWorld(), OwnerCharacter->GetActorLocation() + (OwnerCharacter->GetActorUpVector() * UpRange),
											OwnerCharacter->GetActorLocation() + (OwnerCharacter->GetActorForwardVector() * ForwardRange), 30.0f,
											UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility), true, ActorsToIgnore, EDrawDebugTrace::None, 
											OUT ClimbUpTraceResult, true);

	return ClimbUpTraceResult;
}

// Get hit result of surface below the player
FHitResult UClimbingComponent::DropDownTrace()
{
	FHitResult DropDownTraceResult;

	FVector StartLocation = OwnerCharacter->GetActorLocation() + (OwnerCharacter->GetActorUpVector() * -1.0f * (OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 25.0f));
	FVector EndLocation = StartLocation + (OwnerCharacter->GetActorForwardVector() * -100.0f);

	UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(), StartLocation, EndLocation, 30.0f, Ledges, 
													  true, ActorsToIgnore, EDrawDebugTrace::None, OUT DropDownTraceResult, true);

	return DropDownTraceResult;
}

// Get hit result to see if the player can shimmy to either side
FHitResult UClimbingComponent::SideShimmyTrace(float UpRange, float SideRange)
{
	FHitResult SideShimmyTraceResult;

	UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), OwnerCharacter->GetActorLocation(), 
													OwnerCharacter->GetActorLocation() + ((OwnerCharacter->GetActorUpVector() * UpRange) + (OwnerCharacter->GetActorRightVector() * SideRange)),
													Ledges, true, ActorsToIgnore, EDrawDebugTrace::None, OUT SideShimmyTraceResult, true);

	return SideShimmyTraceResult;
}

// Get hit result to see if there is a corner the player to traverse
FHitResult UClimbingComponent::CornerTrace(FName SocketName, float ForwardRange, float SideRange)
{
	FHitResult CornerTraceResult;

	FVector StartLocation = OwnerCharacter->GetMesh()->GetSocketLocation(SocketName) + (OwnerCharacter->GetActorForwardVector() * ForwardRange);
	FVector EndLocation = StartLocation + (OwnerCharacter->GetActorRightVector() * SideRange);

	UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(), StartLocation, EndLocation, Ledges, true, ActorsToIgnore, EDrawDebugTrace::None, OUT CornerTraceResult, true);

	return CornerTraceResult;
}

void UClimbingComponent::GrabLedge(FVector TargetLocation, AActor* CurrentLedge, bool bIsLedgeDrop)
{
	// Set variables
	OwnerCharacter->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OwnerCharacter->GetSpringArmComponent()->bDoCollisionTest = false;

	// Information for ledge grab
	FVector GrabLocation = TargetLocation - (OwnerCharacter->GetActorForwardVector() * 5.0f);
	FLatentActionInfo LatentActionInfo;
	LatentActionInfo.CallbackTarget = OwnerCharacter;
	FRotator GrabRotation = (CurrentLedge->GetActorForwardVector() * -1.0f).Rotation();

	// Limit camera rotation
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMin = GrabRotation.Yaw - 45.0f;
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMax = GrabRotation.Yaw + 45.0f;

	// Move the player towards the ledge
	if (bIsLedgeDrop)
	{
		GrabLocation = TargetLocation + (OwnerCharacter->GetActorForwardVector() * 5.0f);

		OwnerCharacter->PlayAnimMontage(DropToLedgeAnimation, 1.0f, TEXT("None"));

		UKismetSystemLibrary::MoveComponentTo(OwnerCharacter->GetRootComponent(), FVector(GrabLocation.X, GrabLocation.Y, CurrentLedge->GetActorLocation().Z - 115.0f), 
											  GrabRotation, true, true, DropToLedgeAnimation->SequenceLength, true, EMoveComponentAction::Move, LatentActionInfo);
	}
	else
	{
		UKismetSystemLibrary::MoveComponentTo(OwnerCharacter->GetRootComponent(), FVector(GrabLocation.X, GrabLocation.Y, CurrentLedge->GetActorLocation().Z - 115.0f),
											  GrabRotation, true, true, 0.1f, true, EMoveComponentAction::Move, LatentActionInfo);
	}

	// Add a delay to enable input
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]()
	{
		OwnerCharacter->EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	}, 0.5f, false);
}

void UClimbingComponent::ReleaseLedge()
{
	// Return camera rotation to original values
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMin = OriginalYawMin;
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->PlayerCameraManager->ViewYawMax = OriginalYawMax;

	// Reset variables
	OwnerCharacter->GetSpringArmComponent()->bDoCollisionTest = true;
	OwnerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = true;
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	OwnerCharacter->UnCrouch();
}

void UClimbingComponent::ClimbLedge()
{
	// Alow the player to climb a ledge if there is a flat surface above them
	if (FVector::DotProduct(ClimbUpTrace(200.0f, 150.0f).ImpactNormal, OwnerCharacter->GetActorUpVector()) == 1.0f && ClimbLedgeAnimation)
		OwnerCharacter->PlayAnimMontage(ClimbLedgeAnimation, 1.0f, TEXT("None"));
}

void UClimbingComponent::DropToLedge()
{
	// Allow the player to drop to a ledge if they are moving slow enough and facing it enough
	if (OwnerCharacter->GetVelocity().Length() <= MaxWalkOffSpeed && DropDownTrace().bBlockingHit)
	{
		float DotProd = FVector::DotProduct(DropDownTrace().GetActor()->GetActorForwardVector(), OwnerCharacter->GetActorForwardVector());
		if (DotProd <= 1.0f && DotProd > 0.75f)
			GrabLedge(DropDownTrace().ImpactPoint, DropDownTrace().GetActor(), true);
	}
}
