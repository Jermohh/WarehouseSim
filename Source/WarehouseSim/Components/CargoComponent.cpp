#include "CargoComponent.h"
#include "WarehouseSim/Items/Items.h"
#include "WarehouseSim/Items/PalletActor.h"
#include "WarehouseSim/Items/ScannerStation.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"

UCargoComponent::UCargoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCargoComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AWarehouseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("CargoComponent: Failed to get OwnerCharacter from GetOwner()"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CargoComponent: Initialized with %s"), *OwnerCharacter->GetName());
	}
}

void UCargoComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCargoComponent, HeldItem);
	DOREPLIFETIME(UCargoComponent, HeldPallet);
}

/* ----------------- ITEM HANDLING ----------------- */

void UCargoComponent::PickupItem(AItems* Item)
{
	if (!Item || HeldItem || !OwnerCharacter || Item->IsHeld()) return;

	Item->HoldingCharacter = OwnerCharacter;
	Item->DroppedByCharacter = OwnerCharacter;

	// If item was on a pallet, free its slot
	if (Item->PlacedOnPallet && Item->AssignedPalletSlotIndex != INDEX_NONE)
	{
		Item->PlacedOnPallet->MarkSlotEmpty(Item->AssignedPalletSlotIndex);
		Item->AssignedPalletSlotIndex = INDEX_NONE;
		Item->PlacedOnPallet = nullptr;
	}

	// Only local controller triggers pickup
	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC || !PC->IsLocalController()) return;

	HeldItem = Item;
	LastHeldItem = Item;
	Item->bWasPlacedOnPallet = false;

	// Disable physics and collisions
	Item->SetActorEnableCollision(false);
	Item->ItemMesh->SetSimulatePhysics(false);
	Item->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Item->ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	// Attach to FP camera
	if (UCameraComponent* Camera = OwnerCharacter->GetFirstPersonCamera())
	{
		Item->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Item->SetActorRelativeLocation(Item->FirstPersonItemOffset);
		Item->SetActorRelativeRotation(Item->FirstPersonItemRotation);
	}

	ServerPickupItem(Item);
}

void UCargoComponent::DropItem()
{
	if (!HeldItem || !OwnerCharacter) return;

	if (OwnerCharacter->HasAuthority())
	{
		LastHeldItem = HeldItem;
		AItems* ItemToDrop = HeldItem;

		ItemToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// Place in front of character
		const FVector DropLocation = OwnerCharacter->GetActorLocation() +
			OwnerCharacter->GetActorForwardVector() * 100.f + FVector(0.f, 0.f, 50.f);
		ItemToDrop->SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);

		// Re-enable physics
		ItemToDrop->SetActorEnableCollision(true);
		ItemToDrop->ItemMesh->SetSimulatePhysics(true);
		ItemToDrop->ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemToDrop->ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);

		ItemToDrop->HoldingCharacter = nullptr;
		ItemToDrop->Dropped();

		MulticastOnItemDropped(ItemToDrop);
		HeldItem = nullptr;
	}
	else
	{
		ServerDropItem();
	}
}

void UCargoComponent::ServerPickupItem_Implementation(AItems* Item)
{
	if (!Item || HeldItem || !OwnerCharacter) return;

	Item->HoldingCharacter = OwnerCharacter;
	Item->DroppedByCharacter = OwnerCharacter;
	Item->SetOwner(OwnerCharacter);

	// Clear from pallet if needed
	if (Item->PlacedOnPallet && Item->AssignedPalletSlotIndex != INDEX_NONE)
	{
		Item->PlacedOnPallet->MarkSlotEmpty(Item->AssignedPalletSlotIndex);
		Item->AssignedPalletSlotIndex = INDEX_NONE;
		Item->PlacedOnPallet = nullptr;
	}

	HeldItem = Item;
	MulticastAttachHeldItem(Item);
}

void UCargoComponent::ServerDropItem_Implementation()
{
	if (!HeldItem || !OwnerCharacter) return;

	AItems* ItemToDrop = HeldItem;
	ItemToDrop->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	const FVector DropLocation = OwnerCharacter->GetActorLocation() +
		OwnerCharacter->GetActorForwardVector() * 100.f + FVector(0.f, 0.f, 50.f);
	ItemToDrop->SetActorLocation(DropLocation, false, nullptr, ETeleportType::TeleportPhysics);

	// Physics reset
	ItemToDrop->SetActorEnableCollision(true);
	ItemToDrop->ItemMesh->SetSimulatePhysics(true);
	ItemToDrop->ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemToDrop->ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);

	ItemToDrop->DroppedByCharacter = OwnerCharacter;
	ItemToDrop->Dropped();

	MulticastOnItemDropped(ItemToDrop);
	HeldItem = nullptr;
}

void UCargoComponent::MulticastAttachHeldItem_Implementation(AItems* ItemToAttach)
{
	if (!ItemToAttach || !OwnerCharacter) return;

	ItemToAttach->SetActorEnableCollision(false);
	ItemToAttach->ItemMesh->SetSimulatePhysics(false);
	ItemToAttach->ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UCameraComponent* Camera = OwnerCharacter->GetFirstPersonCamera())
	{
		ItemToAttach->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		ItemToAttach->SetActorRelativeLocation(ItemToAttach->FirstPersonItemOffset);
		ItemToAttach->SetActorRelativeRotation(ItemToAttach->FirstPersonItemRotation);
	}
}

void UCargoComponent::MulticastOnItemDropped_Implementation(AItems* DroppedItem)
{
	if (DroppedItem) DroppedItem->Dropped();
}

/* ----------------- PALLET HANDLING ----------------- */

void UCargoComponent::PickupPallet(APalletActor* Pallet)
{
	if (!Pallet || HeldItem || !OwnerCharacter) return;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC || !PC->IsLocalController()) return;

	Pallet->SetActorEnableCollision(false);
	Pallet->PalletMesh->SetSimulatePhysics(false);

	if (UCameraComponent* Camera = OwnerCharacter->GetFirstPersonCamera())
	{
		Pallet->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Pallet->SetActorRelativeLocation(FirstPersonPalletOffset);
		Pallet->SetActorRelativeRotation(FirstPersonPalletRotation);
	}

	ServerPickupPallet(Pallet);
}

void UCargoComponent::ServerPickupPallet_Implementation(APalletActor* Pallet)
{
	if (!Pallet || HeldItem || !OwnerCharacter) return;
	HeldPallet = Pallet;
	MulticastPickupPallet(Pallet);
}

void UCargoComponent::MulticastPickupPallet_Implementation(APalletActor* Pallet)
{
	if (!Pallet || !OwnerCharacter) return;

	Pallet->SetActorEnableCollision(false);
	Pallet->PalletMesh->SetSimulatePhysics(false);

	if (UCameraComponent* Camera = OwnerCharacter->GetFirstPersonCamera())
	{
		Pallet->AttachToComponent(Camera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Pallet->SetActorRelativeLocation(FirstPersonPalletOffset);
		Pallet->SetActorRelativeRotation(FirstPersonPalletRotation);
	}
}

void UCargoComponent::DropPallet()
{
	if (!HeldPallet || !OwnerCharacter) return;
	if (OwnerCharacter->HasAuthority())
	{
		MulticastDropPallet();
		HeldPallet = nullptr;
	}
	else
	{
		ServerDropPallet();
	}
}

void UCargoComponent::ServerDropPallet_Implementation()
{
	if (!HeldPallet || !OwnerCharacter) return;
	MulticastDropPallet();
	HeldPallet = nullptr;
}

void UCargoComponent::MulticastDropPallet_Implementation()
{
	if (!HeldPallet || !OwnerCharacter) return;

	HeldPallet->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	HeldPallet->SetActorEnableCollision(true);
	HeldPallet->PalletMesh->SetSimulatePhysics(true);
	HeldPallet->PalletMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeldPallet->PalletMesh->SetCollisionResponseToAllChannels(ECR_Block);
}

/* ----------------- PALLET PLACEMENT ----------------- */

void UCargoComponent::TryPlaceItemOnPallet()
{
	if (!HeldItem || !OwnerCharacter) return;

	FHitResult Hit;
	const FVector Start = OwnerCharacter->GetFirstPersonCamera()->GetComponentLocation();
	const FVector End = Start + OwnerCharacter->GetFirstPersonCamera()->GetForwardVector() * 300.f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	Params.AddIgnoredActor(HeldItem);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (APalletActor* HitPallet = Cast<APalletActor>(Hit.GetActor()))
		{
			ServerPlaceItemOnPallet(HitPallet);
		}
	}
}

void UCargoComponent::ServerPlaceItemOnPallet_Implementation(APalletActor* TargetPallet)
{
	if (!HeldItem || !TargetPallet) return;

	USceneComponent* Slot = TargetPallet->GetNextAvailableSlot();
	if (!Slot) return;

	const int32 SlotIndex = TargetPallet->ItemSlots.Find(Slot);
	if (SlotIndex == INDEX_NONE) return;

	TargetPallet->MarkSlotOccupied(SlotIndex);
	HeldItem->AssignedPalletSlotIndex = SlotIndex;
	HeldItem->PlacedOnPallet = TargetPallet;
	HeldItem->bWasPlacedOnPallet = true;

	MulticastPlaceItemOnPallet(TargetPallet, SlotIndex);
	HeldItem->HoldingCharacter = nullptr;
	HeldItem = nullptr;
}

void UCargoComponent::MulticastPlaceItemOnPallet_Implementation(APalletActor* TargetPallet, int32 SlotIndex)
{
	if (!HeldItem || !TargetPallet) return;

	HeldItem->AttachToComponent(TargetPallet->ItemSlots[SlotIndex], FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	HeldItem->SetActorEnableCollision(true);
	HeldItem->ItemMesh->SetSimulatePhysics(false);
	HeldItem->ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HeldItem->ItemMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	HeldItem->ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

/* ----------------- SCANNER ----------------- */

void UCargoComponent::Server_RequestScan_Implementation(AItems* Item, AScannerStation* Scanner)
{
	if (Scanner && Item && Item->ItemValue == 0)
	{
		Scanner->AssignScannedValue(Item);
	}
}

void UCargoComponent::RequestItemScan(AItems* Item, AScannerStation* Scanner)
{
	if (!Item || !Scanner) return;

	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		Scanner->AssignScannedValue(Item);
	}
	else
	{
		Server_RequestScan(Item, Scanner);
	}
}

/* ----------------- ENGINE ----------------- */

void UCargoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
