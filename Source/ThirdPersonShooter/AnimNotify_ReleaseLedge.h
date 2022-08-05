
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ReleaseLedge.generated.h"

class ABaseCharacter;

UCLASS()
class THIRDPERSONSHOOTER_API UAnimNotify_ReleaseLedge : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UAnimNotify_ReleaseLedge();

protected:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	ABaseCharacter* PlayerRef;
};
