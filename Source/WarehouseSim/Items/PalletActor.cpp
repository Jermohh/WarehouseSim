#include "PalletActor.h"
#include "Components/SkeletalMeshComponent.h"

/* ------------------ Construction ------------------ */

APalletActor::APalletActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PalletMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PalletMesh"));
	RootComponent = PalletMesh;

	PalletMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PalletMesh->SetCollisionObjectType(ECC_WorldStatic);
	PalletMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PalletMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	StorageRoot = CreateDefaultSubobject<USceneComponent>(TEXT("StorageRoot"));
	StorageRoot->SetupAttachment(PalletMesh);
	StorageRoot->SetRelativeLocation(FVector(0.f, 0.f, 50.f));

	ItemSlots.SetNum(TotalSlots);
	SlotOccupied.Init(false, TotalSlots);

	const float SlotSpacing = 50.f;
	int32 SlotIndex = 0;

	for (int32 Row = 0; Row < 2; ++Row)
	{
		for (int32 Col = 0; Col < 4; ++Col)
		{
			if (SlotIndex >= TotalSlots) break;

			FString SlotName = FString::Printf(TEXT("ItemSlot_%d"), SlotIndex);
			USceneComponent* NewSlot = CreateDefaultSubobject<USceneComponent>(*SlotName);
			NewSlot->SetupAttachment(StorageRoot);
			NewSlot->SetRelativeLocation(FVector(Col * SlotSpacing, 0.f, Row * 50.f));

			ItemSlots[SlotIndex] = NewSlot;
			++SlotIndex;
		}
	}
}

/* ------------------ Slots ------------------ */

USceneComponent* APalletActor::GetNextAvailableSlot()
{
	for (int32 i = 0; i < SlotOccupied.Num(); ++i)
	{
		if (!SlotOccupied[i])
		{
			return ItemSlots[i];
		}
	}
	return nullptr;
}

void APalletActor::MarkSlotOccupied(int32 Index)
{
	if (SlotOccupied.IsValidIndex(Index))
	{
		SlotOccupied[Index] = true;
	}
}

void APalletActor::MarkSlotEmpty(int32 Index)
{
	if (SlotOccupied.IsValidIndex(Index))
	{
		SlotOccupied[Index] = false;
	}
}

/* ------------------ Hand IK ------------------ */

FVector APalletActor::GetHandIKSocketLocation() const
{
	return PalletMesh->GetSocketLocation("RightHandPalletSocket");
}

FVector APalletActor::GetLeftHandIKSocketLocation() const
{
	if (PalletMesh && PalletMesh->DoesSocketExist("LeftHandPalletSocket"))
	{
		return PalletMesh->GetSocketLocation("LeftHandPalletSocket");
	}
	return GetActorLocation();
}

/* ------------------ Occupancy Array ------------------ */

void APalletActor::SetSlotOccupiedArray(const TArray<bool>& In)
{
	if (In.Num() == SlotOccupied.Num())
	{
		SlotOccupied = In;
	}
	else
	{
		SlotOccupied.Init(false, TotalSlots);
		const int32 CopyCount = FMath::Min(TotalSlots, In.Num());
		for (int32 i = 0; i < CopyCount; ++i)
		{
			SlotOccupied[i] = In[i];
		}
	}
}