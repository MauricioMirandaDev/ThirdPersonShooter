
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LedgeVolume.generated.h"

class UBoxComponent; 
class ABaseCharacter;

UCLASS()
class THIRDPERSONSHOOTER_API ALedgeVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALedgeVolume();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Overlap functions
	UFUNCTION()
	void BeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void EndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Volume;

	ABaseCharacter* PlayerRef; 
};
