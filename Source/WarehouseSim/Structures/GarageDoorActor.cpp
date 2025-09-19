#include "GarageDoorActor.h"
#include "Net/UnrealNetwork.h"
#include "WarehouseSim/Structures/DoorSirenBase.h"
#include "WarehouseSim/Structures/TruckActor.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"
#include "WarehouseSim/GameState/MainGameState.h"
#include "WarehouseSim/Structures/TruckCountdownWidget.h"
#include "Components/WidgetComponent.h"

AGarageDoorActor::AGarageDoorActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;

	TimerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TimerWidget"));
	TimerWidget->SetupAttachment(RootComponent);
	TimerWidget->SetWidgetSpace(EWidgetSpace::World);
	TimerWidget->SetDrawSize(FVector2D(200.f, 50.f));
	TimerWidget->SetRelativeLocation(FVector(0.f, 0.f, 250.f));
}

void AGarageDoorActor::BeginPlay()
{
	Super::BeginPlay();

	if (TimerWidget && TimerWidget->GetUserWidgetObject())
	{
		if (UTruckCountdownWidget* W = Cast<UTruckCountdownWidget>(TimerWidget->GetUserWidgetObject()))
		{
			W->LinkedDoor = this;
		}
	}
}

void AGarageDoorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		ReplicatedRemainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(DoorOpenTimer);
	}

	if (TimerWidget && TimerWidget->GetUserWidgetObject())
	{
		UTruckCountdownWidget* CountdownWidget = Cast<UTruckCountdownWidget>(TimerWidget->GetUserWidgetObject());
		if (CountdownWidget)
		{
			CountdownWidget->UpdateCountdown(HasAuthority() ? ReplicatedRemainingTime : ReplicatedRemainingTime);
		}
	}
}

void AGarageDoorActor::OpenDoor()
{
	UE_LOG(LogTemp, Warning, TEXT("[DOOR] OpenDoor()  Auth=%d  Unlocked=%d  (Before) Opened=%d"),
		HasAuthority() ? 1 : 0, bDoorUnlocked ? 1 : 0, bDoorOpened ? 1 : 0);

	if (!bDoorUnlocked) return;
	bDoorOpened = true;
	OnRep_DoorOpened();

	if (HasAuthority() && ActiveTruck == nullptr)
	{
		ActiveTruck = GetWorld()->SpawnActor<ATruckActor>(TruckClass, TruckSpawnLocation, TruckSpawnRotation);
		if (ActiveTruck)
		{
			AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();
			int32 ExtraBoxes = (GS ? GS->TruckUpgradeLevel : 0) * 2;

			ActiveTruck->ArriveAndUnload(ExtraBoxes);
		}
	}

	Multicast_TriggerSirenColorChange();

}

void AGarageDoorActor::OnRep_DoorOpened()
{
	if (DoorMesh && bDoorOpened)
	{
		FVector NewLocation = DoorMesh->GetRelativeLocation();
		NewLocation.Z += 400.f; // Raise the door
		DoorMesh->SetRelativeLocation(NewLocation);
	}
	else
	{
		if (DoorMesh)
		{
			FVector NewLocation = DoorMesh->GetRelativeLocation();
			NewLocation.Z -= 400.f;
			DoorMesh->SetRelativeLocation(NewLocation);
		}
		bDoorOpened = false;

		if (DoorSirenRef)
		{
			DoorSirenRef->ResetTriggerColorChange();
		}
	}
}

void AGarageDoorActor::Multicast_TriggerSirenColorChange_Implementation()
{
	if (DoorSirenRef)
	{
		DoorSirenRef->TriggerColorChange();
	}

}

void AGarageDoorActor::CloseDoor()
{
	UE_LOG(LogTemp, Warning, TEXT("[DOOR] CloseDoor()  Auth=%d  (Before) Opened=%d"),
		HasAuthority() ? 1 : 0, bDoorOpened ? 1 : 0);

	if (!bDoorOpened) return;
	if (DoorMesh)
	{
		FVector NewLocation = DoorMesh->GetRelativeLocation();
		NewLocation.Z -= 400.f;
		DoorMesh->SetRelativeLocation(NewLocation);
	}
	bDoorOpened = false;

	if (HasAuthority() && ActiveTruck)
	{
		ActiveTruck->Destroy();
		ActiveTruck = nullptr;
	}

	if (DoorSirenRef)
	{
		DoorSirenRef->ResetTriggerColorChange();
	}
}


void AGarageDoorActor::ResetTimer()
{
	UE_LOG(LogTemp, Warning, TEXT("[DOOR] ResetTimer() Auth=%d  Unlocked=%d  StartIn=%.2fs"),
		HasAuthority() ? 1 : 0, bDoorUnlocked ? 1 : 0, TimeUntilTruckArrives);

	if (!bDoorUnlocked) return;

	GetWorld()->GetTimerManager().ClearTimer(DoorOpenTimer);
	GetWorld()->GetTimerManager().SetTimer(
		DoorOpenTimer,
		this,
		&AGarageDoorActor::OpenDoor,
		TimeUntilTruckArrives,
		false
	);
}

float AGarageDoorActor::GetRemainingTruckTime() const
{
	return GetWorld()->GetTimerManager().GetTimerRemaining(DoorOpenTimer);
}

void AGarageDoorActor::OnRep_RemainingTime()
{
	UE_LOG(LogTemp, Warning, TEXT("CLIENT REMAINING TIME REPLICATED: %f"), ReplicatedRemainingTime);

	if (TimerWidget && TimerWidget->GetUserWidgetObject())
	{
		UTruckCountdownWidget* CountdownWidget = Cast<UTruckCountdownWidget>(TimerWidget->GetUserWidgetObject());
		if (CountdownWidget)
		{
			CountdownWidget->UpdateCountdown(ReplicatedRemainingTime);
		}
	}
}

void AGarageDoorActor::ServerBuyDoor_Implementation(AWarehouseCharacter* Buyer)
{
	if (!Buyer || bDoorUnlocked) return;

	if (Buyer->Money >= DoorCost)
	{
		Buyer->Money -= DoorCost;
		Buyer->UpdateHUDMoney();

		bDoorUnlocked = true;

		GetWorld()->GetTimerManager().SetTimer(
			DoorOpenTimer,
			this,
			&AGarageDoorActor::OpenDoor,
			TimeUntilTruckArrives,
			false
		);
	}
}

void AGarageDoorActor::StartArrivalIn(float Seconds)
{
	UE_LOG(LogTemp, Warning, TEXT("[DOOR] StartArrivalIn(%.2fs) Auth=%d  Unlocked=%d  Opened=%d"),
		Seconds, HasAuthority() ? 1 : 0, bDoorUnlocked ? 1 : 0, bDoorOpened ? 1 : 0);

	if (!bDoorUnlocked || bDoorOpened) return;

	GetWorld()->GetTimerManager().ClearTimer(DoorOpenTimer);
	if (Seconds > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			DoorOpenTimer,
			this,
			&AGarageDoorActor::OpenDoor,
			Seconds,
			false
		);
	}
}

void AGarageDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGarageDoorActor, bDoorOpened);
	DOREPLIFETIME(AGarageDoorActor, bDoorUnlocked);
	DOREPLIFETIME(AGarageDoorActor, ReplicatedRemainingTime);
}

