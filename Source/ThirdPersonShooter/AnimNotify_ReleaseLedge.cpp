
#include "AnimNotify_ReleaseLedge.h"
#include "BaseCharacter.h"
#include "ClimbingComponent.h"
#include "Kismet/GameplayStatics.h"

UAnimNotify_ReleaseLedge::UAnimNotify_ReleaseLedge()
{
	NotifyColor = FColor(50, 50, 255);
	PlayerRef = nullptr;
}

void UAnimNotify_ReleaseLedge::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp->GetOwner()->IsA(ABaseCharacter::StaticClass()))
	{
		PlayerRef = Cast<ABaseCharacter>(MeshComp->GetOwner());
		PlayerRef->GetClimbingComponent()->ReleaseLedge();
	}
}
