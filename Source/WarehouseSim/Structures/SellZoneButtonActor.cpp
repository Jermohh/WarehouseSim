#include "SellZoneButtonActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"
#include "WarehouseSim/Structures/GarageSellDoorActor.h"

/* ------------------ Construction ------------------ */

ASellZoneButtonActor::ASellZoneButtonActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ButtonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ButtonMesh"));
	RootComponent = ButtonMesh;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASellZoneButtonActor::OnOverlapBegin);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ASellZoneButtonActor::OnOverlapEnd);
}

/* ------------------ BeginPlay ------------------ */

void ASellZoneButtonActor::BeginPlay()
{
	Super::BeginPlay();
}

/* ------------------ Overlap Events ------------------ */

void ASellZoneButtonActor::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (AWarehouseCharacter* Char = Cast<AWarehouseCharacter>(OtherActor))
	{
		Char->OverlappingSellZoneButton = this;
	}
}

void ASellZoneButtonActor::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (AWarehouseCharacter* Char = Cast<AWarehouseCharacter>(OtherActor))
	{
		if (Char->OverlappingSellZoneButton == this)
		{
			Char->OverlappingSellZoneButton = nullptr;
		}
	}
}