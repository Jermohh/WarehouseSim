#include "Lights.h"
#include "Components/PointLightComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


ALights::ALights()
{
	PrimaryActorTick.bCanEverTick = true;

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	RootComponent = PointLight;

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootComponent);
	InteractionBox->SetBoxExtent(FVector(100.f));
	InteractionBox->SetCollisionProfileName("Trigger");

	InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &ALights::OnOverlapBegin);
	InteractionBox->OnComponentEndOverlap.AddDynamic(this, &ALights::OnOverlapEnd);
}

void ALights::BeginPlay()
{
	Super::BeginPlay();
}

void ALights::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void ALights::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		bCanToggle = true;
	}
}

void ALights::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this)
	{
		bCanToggle = false;
	}
}

void ALights::ToggleLight()
{
	if (bCanToggle && PointLight)
	{
		PointLight->ToggleVisibility();
	}
}
