#include "GarageSellDoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "WarehouseSim/Items/SellZone.h"
#include "Net/UnrealNetwork.h"
#include "WarehouseSim/Structures/SellingTruckActor.h"

AGarageSellDoorActor::AGarageSellDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	SetRootComponent(DoorMesh);
}

void AGarageSellDoorActor::BeginPlay()
{
	Super::BeginPlay();
	InitialDoorLocation = GetActorLocation();
	TargetDoorLocation = InitialDoorLocation;
}

void AGarageSellDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AGarageSellDoorActor::ActivateSell()
{
	if (HasAuthority())
	{
		bShouldCloseDoor = true;
	}

	CloseDoor();
	GetWorldTimerManager().SetTimer(DoorTimerHandle, this, &AGarageSellDoorActor::OpenDoor, 10.f, false);
}

void AGarageSellDoorActor::OnRep_ShouldCloseDoor()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_ShouldCloseDoor called"));


	if (bShouldCloseDoor)
	{
		CloseDoor();

		if (HasAuthority())
		{
			GetWorldTimerManager().SetTimer(DoorTimerHandle, this, &AGarageSellDoorActor::OpenDoor, 10.f, false);

			bShouldCloseDoor = false;
		}
	}
}

void AGarageSellDoorActor::MulticastPlayCloseDoorTimeline_Implementation()
{
	PlayCloseDoorTimeline();
}

void AGarageSellDoorActor::MulticastPlayOpenDoorTimeline_Implementation()
{
	PlayOpenDoorTimeline();
}

void AGarageSellDoorActor::CloseDoor()
{
	if (HasAuthority())
	{
		MulticastPlayCloseDoorTimeline();
	}
}

void AGarageSellDoorActor::OpenDoor()
{
	if (HasAuthority())
	{
		MulticastPlayOpenDoorTimeline();
	}

}

void AGarageSellDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGarageSellDoorActor, TargetDoorLocation);
	DOREPLIFETIME(AGarageSellDoorActor, bShouldCloseDoor);
}