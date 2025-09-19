#include "TruckActor.h"
#include "WarehouseSim/Items/Items.h"

/* ------------------ Construction ------------------ */

ATruckActor::ATruckActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TruckMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TruckMesh"));
	RootComponent = TruckMesh;
}

/* ------------------ BeginPlay ------------------ */

void ATruckActor::BeginPlay()
{
	Super::BeginPlay();
}

/* ------------------ Actions ------------------ */

void ATruckActor::ArriveAndUnload(int32 ExtraBoxes)
{
	SpawnBoxes(ExtraBoxes);
}

/* ------------------ Box Spawning ------------------ */

void ATruckActor::SpawnBoxes(int32 ExtraBoxes)
{
	const FVector Origin = GetActorLocation() + BoxSpawnOffset;
	const int32 TotalBoxes = BoxesToDeliver + ExtraBoxes;

	for (int32 i = 0; i < TotalBoxes; ++i)
	{
		int32 Row = i / BoxesPerRow;
		int32 Col = i % BoxesPerRow;

		float OffsetX = Row * BoxSpacingX;
		float OffsetY = Col * BoxSpacingY;

		FVector SpawnLocation = Origin + FVector(OffsetX, OffsetY, 0.f);

		GetWorld()->SpawnActor<AItems>(BoxClass, SpawnLocation, FRotator::ZeroRotator);
		DrawDebugSphere(GetWorld(), SpawnLocation, 10.f, 12, FColor::Red, false, 10.f);
	}
}

void ATruckActor::SetExtraBoxes(int32 ExtraBoxes)
{
	BoxesToDeliver += ExtraBoxes;
}