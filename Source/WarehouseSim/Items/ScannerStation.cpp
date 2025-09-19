#include "ScannerStation.h"
#include "Components/BoxComponent.h"
#include "WarehouseSim/Items/Items.h"
#include "Net/UnrealNetwork.h"
#include "WarehouseSim/Components/CargoComponent.h"
#include "WarehouseSim/GameState/MainGameState.h"
#include "Camera/CameraComponent.h"

/* ------------------ Construction ------------------ */

AScannerStation::AScannerStation()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	ScannerTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("ScannerTrigger"));
	RootComponent = ScannerTrigger;

	ScannerTrigger->SetBoxExtent(FVector(100.f));
	ScannerTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ScannerTrigger->SetCollisionResponseToAllChannels(ECR_Overlap);
	ScannerTrigger->SetCollisionObjectType(ECC_WorldDynamic);

	// Physics defaults (disabled when mounted/picked up)
	ScannerTrigger->SetSimulatePhysics(true);
	ScannerTrigger->SetEnableGravity(true);
}

/* ------------------ BeginPlay ------------------ */

void AScannerStation::BeginPlay()
{
	Super::BeginPlay();

	if (ScannerTrigger)
	{
		ScannerTrigger->OnComponentBeginOverlap.AddDynamic(this, &AScannerStation::OnScannerOverlap);
	}
}

/* ------------------ Overlap -> Scan ------------------ */

void AScannerStation::OnScannerOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	AItems* Item = Cast<AItems>(OtherActor);
	if (!Item || Item->ItemValue != 0) return;

	if (HasAuthority())
	{
		AssignScannedValue(Item);
	}
	else
	{
		if (UCargoComponent* Cargo = Item->GetOwningCargoComponent())
		{
			Cargo->RequestItemScan(Item, this);
		}
	}
}

/* ------------------ Scan Logic ------------------ */

void AScannerStation::AssignScannedValue(AItems* Item)
{
	float Roll = FMath::FRand();
	int32 Value = 0;

	if (Roll < 0.5f)       Value = 10;  // 50%
	else if (Roll < 0.8f)  Value = 20;  // 30%
	else                   Value = 50;  // 20%

	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		if (GS->ScannerBonusMaxPct > 0.f)
		{
			const float BonusPct = FMath::FRandRange(GS->ScannerBonusMinPct, GS->ScannerBonusMaxPct);
			const int32 BonusValue = FMath::RoundToInt(Value * BonusPct);
			Value += BonusValue;
		}
	}

	int32 RandomCategory = FMath::RandRange(1, 4);
	if (RandomCategory <= 0 || RandomCategory > 4)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid random category %d"), RandomCategory);
		RandomCategory = 1;
	}

	Item->SetCategory(RandomCategory);
	Item->SetItemValue(Value);
	Item->MulticastApplyColor();

	if (const AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		UE_LOG(LogTemp, Warning, TEXT(
			"Scanner assigned value %d (+bonus %.0f%%..%.0f%%) and category %d to item: %s"),
			Value,
			GS->ScannerBonusMinPct * 100.f,
			GS->ScannerBonusMaxPct * 100.f,
			RandomCategory,
			*Item->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(
			"Scanner assigned value %d (no bonus) and category %d to item: %s"),
			Value,
			RandomCategory,
			*Item->GetName());
	}
}

/* ------------------ RPCs ------------------ */

void AScannerStation::Server_RequestScan_Implementation(AItems* Item)
{
	if (!Item || Item->ItemValue != 0) return;

	UE_LOG(LogTemp, Warning, TEXT("ServerRequestScanShouldBeCalled"));
	AssignScannedValue(Item);
}

void AScannerStation::Server_AttachToMount_Implementation(USceneComponent* Mount)
{
	if (!IsValid(Mount)) return;

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootPrim->SetSimulatePhysics(false);
		RootPrim->SetEnableGravity(false);
		RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		RootPrim->SetCollisionResponseToAllChannels(ECR_Overlap);
	}

	AttachToComponent(Mount, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(FVector::ZeroVector);
	SetActorRelativeRotation(FRotator::ZeroRotator);
}

void AScannerStation::Multicast_OnPickedUp_Implementation(UCameraComponent* AttachCamera)
{
	if (!AttachCamera) return;

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootPrim->SetSimulatePhysics(false);
		RootPrim->SetEnableGravity(false);
		RootPrim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	AttachToComponent(AttachCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(FVector(60.f, 0.f, -10.f));
	SetActorRelativeRotation(FRotator::ZeroRotator);
}

void AScannerStation::Multicast_OnDropped_Implementation()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RootPrim->SetCollisionObjectType(ECC_WorldDynamic);
		RootPrim->SetCollisionResponseToAllChannels(ECR_Overlap);
		RootPrim->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		RootPrim->SetEnableGravity(true);
		RootPrim->SetSimulatePhysics(true);
		RootPrim->WakeAllRigidBodies();
	}
}