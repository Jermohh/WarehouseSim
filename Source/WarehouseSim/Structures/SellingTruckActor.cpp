#include "SellingTruckActor.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

/* ------------------ Construction ------------------ */

ASellingTruckActor::ASellingTruckActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TruckMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TruckMesh"));
	RootComponent = TruckMesh;
}

/* ------------------ BeginPlay ------------------ */

void ASellingTruckActor::BeginPlay()
{
	Super::BeginPlay();
}

/* ------------------ Actions ------------------ */

void ASellingTruckActor::ActivateTruck()
{
	if (HasAuthority() && !bTruckTriggered)
	{
		bTruckTriggered = true;
		Multicast_TriggerSellingTruck();

		GetWorldTimerManager().SetTimer(
			ReturnTruckTimerHandle,
			this,
			&ASellingTruckActor::HandleReturnTruck,
			ReturnDelay,
			false
		);
	}
}

void ASellingTruckActor::TriggerReturnTruck()
{
	if (HasAuthority())
	{
		bTruckTriggered = false;
		Multicast_ReturnTruck();
	}
}

/* ------------------ RPCs ------------------ */

void ASellingTruckActor::Multicast_TriggerSellingTruck_Implementation()
{
	DriveTruckAway();
}

void ASellingTruckActor::Multicast_ReturnTruck_Implementation()
{
	ReturnTruck();
}

/* ------------------ Replication ------------------ */

void ASellingTruckActor::OnRep_TruckTriggered()
{
	if (bTruckTriggered)
	{
		DriveTruckAway();
	}
}

void ASellingTruckActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASellingTruckActor, bTruckTriggered);
}

/* ------------------ Timer ------------------ */

void ASellingTruckActor::HandleReturnTruck()
{
	TriggerReturnTruck();
}