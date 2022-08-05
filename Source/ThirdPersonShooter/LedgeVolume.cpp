
#include "LedgeVolume.h"
#include "BaseCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ALedgeVolume::ALedgeVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a default root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create a box component and set default values
	Volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Volume->SetupAttachment(Root);
	Volume->SetBoxExtent(FVector(10.0f, 100.0f, 10.0f), true);
	Volume->SetGenerateOverlapEvents(true);
	Volume->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	Volume->SetCollisionProfileName(TEXT("LedgePreset"), true);
	Volume->OnComponentBeginOverlap.AddDynamic(this, &ALedgeVolume::BeginOverlap);
	Volume->OnComponentEndOverlap.AddDynamic(this, &ALedgeVolume::EndOverlap);
}

// Called when the game starts or when spawned
void ALedgeVolume::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = Cast<ABaseCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!PlayerRef)
		return;
}

// Called every frame
void ALedgeVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALedgeVolume::BeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(ABaseCharacter::StaticClass()))
	{
		float DotProd = FVector::DotProduct(PlayerRef->GetActorForwardVector(), GetActorForwardVector());
		if (DotProd > -1.0f && DotProd < -0.75f)
		{
			FHitResult Target = PlayerRef->ForwardTrace(100.0f);
			PlayerRef->LedgeGrab(this, Target.ImpactPoint, false);
		}
		
	}
}

void ALedgeVolume::EndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

