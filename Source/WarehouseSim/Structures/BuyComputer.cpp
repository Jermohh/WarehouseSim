#include "BuyComputer.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"
#include "Net/UnrealNetwork.h"

/* ------------------ Construction ------------------ */

ABuyComputer::ABuyComputer()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ComputerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ComputerMesh"));
	SetRootComponent(ComputerMesh);

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(RootComponent);
	OverlapBox->SetBoxExtent(FVector(150.f));
	OverlapBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

/* ------------------ BeginPlay ------------------ */

void ABuyComputer::BeginPlay()
{
	Super::BeginPlay();

	OverlapBox->OnComponentBeginOverlap.AddDynamic(this, &ABuyComputer::OnOverlapBegin);
	OverlapBox->OnComponentEndOverlap.AddDynamic(this, &ABuyComputer::OnOverlapEnd);
}

/* ------------------ Overlap Events ------------------ */

void ABuyComputer::OnOverlapBegin(
	UPrimitiveComponent*,
	AActor* OtherActor,
	UPrimitiveComponent*,
	int32,
	bool,
	const FHitResult&)
{
	if (AWarehouseCharacter* Char = Cast<AWarehouseCharacter>(OtherActor))
	{
		Char->bInRangeOfComputer = true;
	}
}

void ABuyComputer::OnOverlapEnd(
	UPrimitiveComponent*,
	AActor* OtherActor,
	UPrimitiveComponent*,
	int32)
{
	if (AWarehouseCharacter* Char = Cast<AWarehouseCharacter>(OtherActor))
	{
		Char->bInRangeOfComputer = false;
	}
}