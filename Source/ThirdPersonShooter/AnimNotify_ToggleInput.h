
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ToggleInput.generated.h"

class ABaseCharacter;

UCLASS()
class THIRDPERSONSHOOTER_API UAnimNotify_ToggleInput : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UAnimNotify_ToggleInput();

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	UPROPERTY(EditAnywhere)
	bool bCanUseInput;

	ABaseCharacter* PlayerRef; 
};
