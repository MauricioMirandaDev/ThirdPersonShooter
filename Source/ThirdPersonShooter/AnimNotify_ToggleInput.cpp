
#include "AnimNotify_ToggleInput.h"
#include "BaseCharacter.h"
#include "Kismet/GameplayStatics.h"

UAnimNotify_ToggleInput::UAnimNotify_ToggleInput()
{
	NotifyColor = FColor(255, 50, 50);
	bCanUseInput = true;
	PlayerRef = nullptr;
}

void UAnimNotify_ToggleInput::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (MeshComp->GetOwner()->IsA(ABaseCharacter::StaticClass()))
	{
		PlayerRef = Cast<ABaseCharacter>(MeshComp->GetOwner());

		if (PlayerRef && bCanUseInput)
			PlayerRef->EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		else if (PlayerRef && !bCanUseInput)
			PlayerRef->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	}
}
