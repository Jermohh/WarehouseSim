#include "ConveyorBelt.h"
#include "WarehouseSim/Items/Items.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "WarehouseSim/GameState/MainGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

float AConveyorBelt::ConveyorSpeedMultiplier = 1.f;

/* ------------------ Construction ------------------ */

AConveyorBelt::AConveyorBelt()
{
	PrimaryActorTick.bCanEverTick = true;

	BeltMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeltMesh"));
	RootComponent = BeltMesh;

	BeltMesh->SetSimulatePhysics(true);
	BeltMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BeltMesh->SetCollisionResponseToAllChannels(ECR_Block);
	BeltMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	BeltMesh->SetCollisionObjectType(ECC_PhysicsBody);

	SnapPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SnapPoint"));
	SnapPoint->SetupAttachment(BeltMesh);
	SnapPoint->SetRelativeLocation(FVector(100.f, 0.f, 0.f));

	auto CreateSnapBox = [&](FName Name, FVector RelativeLocation) -> UBoxComponent*
		{
			UBoxComponent* SnapBox = CreateDefaultSubobject<UBoxComponent>(Name);
			SnapBox->SetupAttachment(BeltMesh);
			SnapBox->SetBoxExtent(FVector(10.f, 30.f, 30.f));
			SnapBox->SetRelativeLocation(RelativeLocation);
			SnapBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SnapBox->SetCollisionResponseToAllChannels(ECR_Ignore);
			SnapBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
			SnapBox->SetGenerateOverlapEvents(false);
			return SnapBox;
		};

	SnapFront = CreateSnapBox("SnapFront", FVector(100.f, 0.f, 0.f));
	SnapBack = CreateSnapBox("SnapBack", FVector(-100.f, 0.f, 0.f));
	SnapLeft = CreateSnapBox("SnapLeft", FVector(0.f, -100.f, 0.f));
	SnapRight = CreateSnapBox("SnapRight", FVector(0.f, 100.f, 0.f));
	SnapFrontLeft = CreateSnapBox("SnapFrontLeft", FVector(100.f, -100.f, 0.f));
	SnapFrontRight = CreateSnapBox("SnapFrontRight", FVector(100.f, 100.f, 0.f));

	MoveZone = CreateDefaultSubobject<UBoxComponent>(TEXT("MoveZone"));
	MoveZone->SetupAttachment(BeltMesh);
	MoveZone->SetBoxExtent(FVector(100.f, 100.f, 50.f));
	MoveZone->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	MoveZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MoveZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	MoveZone->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	MoveZone->SetGenerateOverlapEvents(true);

	ScannerMountCenter = CreateDefaultSubobject<USceneComponent>(TEXT("ScannerMountCenter"));
	ScannerMountCenter->SetupAttachment(BeltMesh);
	ScannerMountCenter->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	ScannerMountCenter->SetRelativeRotation(FRotator::ZeroRotator);

	ScannerSnapCenter = CreateDefaultSubobject<UBoxComponent>(TEXT("ScannerSnapCenter"));
	ScannerSnapCenter->SetupAttachment(BeltMesh);
	ScannerSnapCenter->SetBoxExtent(FVector(12.f, 12.f, 12.f));
	ScannerSnapCenter->SetRelativeLocation(ScannerMountCenter->GetRelativeLocation());
	ScannerSnapCenter->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ScannerSnapCenter->SetCollisionResponseToAllChannels(ECR_Ignore);
	ScannerSnapCenter->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ScannerSnapCenter->SetGenerateOverlapEvents(false);
}

/* ------------------ BeginPlay ------------------ */

void AConveyorBelt::BeginPlay()
{
	Super::BeginPlay();

	for (UBoxComponent* Zone : MoveZones)
	{
		if (Zone)
		{
			Zone->SetGenerateOverlapEvents(true);
			Zone->UpdateOverlaps();
		}
	}
}

/* ------------------ Tick ------------------ */

void AConveyorBelt::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	MoveBoxes(DeltaTime);
}

/* ------------------ Movement ------------------ */

void AConveyorBelt::MoveBoxes(float DeltaTime)
{
	if (!HasAuthority() || !MoveZone) return;

	AGameStateBase* GS = UGameplayStatics::GetGameState(this);
	float SpeedMultiplier = 1.f;

	if (AMainGameState* MainGS = Cast<AMainGameState>(GS))
	{
		SpeedMultiplier = MainGS->ConveyorSpeedMultiplier;
	}

	TArray<AActor*> Overlapping;
	MoveZone->GetOverlappingActors(Overlapping, AItems::StaticClass());

	for (AActor* Box : Overlapping)
	{
		if (IsValid(Box))
		{
			FVector MoveDir = GetActorForwardVector();
			FVector NewLocation = Box->GetActorLocation() + MoveDir * ConveyorSpeed * SpeedMultiplier * DeltaTime;
			Box->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

/* ------------------ Snapping ------------------ */

UBoxComponent* AConveyorBelt::GetClosestSnapBox(const FVector& HitLocation) const
{
	TArray<UBoxComponent*> SnapBoxes = { SnapFront, SnapBack, SnapLeft, SnapRight, SnapFrontLeft, SnapFrontRight };

	UBoxComponent* Closest = nullptr;
	float ClosestDist = FLT_MAX;

	for (UBoxComponent* Box : SnapBoxes)
	{
		if (!Box) continue;

		float Dist = FVector::DistSquared(Box->GetComponentLocation(), HitLocation);
		if (Dist < ClosestDist)
		{
			Closest = Box;
			ClosestDist = Dist;
		}
	}

	return Closest;
}