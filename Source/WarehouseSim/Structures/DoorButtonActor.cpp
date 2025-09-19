#include "DoorButtonActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "WarehouseSim/Structures/GarageDoorActor.h"
#include "GameFramework/Character.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"

/* ------------------ Construction ------------------ */

ADoorButtonActor::ADoorButtonActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	RootComponent = ButtonMesh;

	BoxSphere = CreateDefaultSubobject<UBoxComponent>(TEXT("ProximitySphere"));
	BoxSphere->SetupAttachment(RootComponent);
	BoxSphere->SetBoxExtent(FVector(150.f, 150.f, 100.f));
	BoxSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	HoverWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HoverWidget"));
	HoverWidget->SetHiddenInGame(true);
	HoverWidget->SetupAttachment(RootComponent);
	HoverWidget->SetWidgetSpace(EWidgetSpace::World);
	HoverWidget->SetDrawSize(FVector2D(600, 600));
	HoverWidget->SetWorldScale3D(FVector(0.3f));
	HoverWidget->SetPivot(FVector2D(0.5f, 0.5f));
}

/* ------------------ BeginPlay ------------------ */

void ADoorButtonActor::BeginPlay()
{
	Super::BeginPlay();

	BoxSphere->OnComponentBeginOverlap.AddDynamic(this, &ADoorButtonActor::OnOverlapBegin);
	BoxSphere->OnComponentEndOverlap.AddDynamic(this, &ADoorButtonActor::OnOverlapEnd);
}

/* ------------------ Door Request ------------------ */

void ADoorButtonActor::RequestDoorClose()
{
	if (LinkedGarageDoor && LinkedGarageDoor->IsDoorOpened())
	{
		UE_LOG(LogTemp, Warning, TEXT("IFCHECKRequestDoorIsBeingCalled"));
		LinkedGarageDoor->CloseDoor();
		LinkedGarageDoor->ResetTimer();
	}
}

/* ------------------ Overlap Events ------------------ */

void ADoorButtonActor::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("overlappingwithbutton"));

	ACharacter* OverlappingCharacter = Cast<ACharacter>(OtherActor);
	if (OverlappingCharacter && OverlappingCharacter->IsPlayerControlled() && OverlappingCharacter->IsLocallyControlled())
	{
		HoverWidget->SetHiddenInGame(false);

		if (AWarehouseCharacter* WarehouseChar = Cast<AWarehouseCharacter>(OverlappingCharacter))
		{
			WarehouseChar->OverlappingDoorButton = this;
		}
	}
}

void ADoorButtonActor::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ACharacter* OverlappingCharacter = Cast<ACharacter>(OtherActor);
	if (OverlappingCharacter && OverlappingCharacter->IsPlayerControlled() && OverlappingCharacter->IsLocallyControlled())
	{
		HoverWidget->SetHiddenInGame(true);

		if (AWarehouseCharacter* WarehouseChar = Cast<AWarehouseCharacter>(OverlappingCharacter))
		{
			if (WarehouseChar->OverlappingDoorButton == this)
			{
				WarehouseChar->OverlappingDoorButton = nullptr;
			}
		}
	}
}