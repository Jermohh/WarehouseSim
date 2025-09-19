#include "Items.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "WarehouseSim/Items/SellZone.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"
#include "WarehouseSim/Components/CargoComponent.h"

AItems::AItems()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	Category = 0;

	// Mesh setup
	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetCollisionObjectType(ECC_PhysicsBody);
	ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->SetGenerateOverlapEvents(true);
	ItemMesh->SetMassOverrideInKg(NAME_None, 1000.f, true);

	// Randomized value
	Value = FMath::RandRange(10, 100);
}

void AItems::BeginPlay()
{
	Super::BeginPlay();
}

void AItems::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/* ------------------ Overlap Events ------------------ */

void AItems::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AItems::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

/* ------------------ Hand IK ------------------ */

FVector AItems::GetHandIKSocketLocation() const
{
	return ItemMesh->GetSocketLocation("RightHandItemSocket");
}

FVector AItems::GetLeftHandIKSocketLocation() const
{
	if (ItemMesh && ItemMesh->DoesSocketExist("LeftHandItemSocket"))
	{
		return ItemMesh->GetSocketLocation("LeftHandItemSocket");
	}

	return GetActorLocation();
}

/* ------------------ Replication ------------------ */

void AItems::OnRep_ItemValue()
{
	UE_LOG(LogTemp, Warning, TEXT("Box Value Changed: %d"), ItemValue);
}

void AItems::OnRep_Category()
{
	ApplyColorForCategory();
	OnCategoryChanged(Category);
}

void AItems::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItems, HoldingCharacter);
	DOREPLIFETIME(AItems, Value);
	DOREPLIFETIME(AItems, bWasPlacedOnPallet);
	DOREPLIFETIME(AItems, ItemValue);
	DOREPLIFETIME(AItems, Category);
}

/* ------------------ Item Value ------------------ */

void AItems::SetItemValue(int32 NewValue)
{
	if (HasAuthority())
	{
		ItemValue = NewValue;
		OnRep_ItemValue();
	}
}

/* ------------------ Category ------------------ */

void AItems::SetCategory(int32 NewCategory)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Items] SetCategory called with %d on item %s"), NewCategory, *GetName());

		Category = NewCategory;
		ApplyColorForCategory();
		OnRep_Category();
	}
}

void AItems::ApplyColorForCategory()
{
	if (!ItemMesh) return;

	UMaterialInterface* TargetMaterial = nullptr;

	switch (Category)
	{
	case 1: TargetMaterial = RedMaterial; break;
	case 2: TargetMaterial = BlueMaterial; break;
	case 3: TargetMaterial = GreenMaterial; break;
	case 4: TargetMaterial = YellowMaterial; break;
	default: break;
	}

	if (TargetMaterial)
	{
		ItemMesh->SetMaterial(0, TargetMaterial);
		UE_LOG(LogTemp, Log, TEXT("[Items] Applied material for Category %d on item %s"), Category, *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Items] No material assigned for Category %d on item %s"), Category, *GetName());
	}
}

void AItems::MulticastApplyColor_Implementation()
{
	ApplyColorForCategory();
}

/* ------------------ Cargo ------------------ */

UCargoComponent* AItems::GetOwningCargoComponent() const
{
	if (HoldingCharacter)
	{
		return HoldingCharacter->FindComponentByClass<UCargoComponent>();
	}
	return nullptr;
}

/* ------------------ Item Handling ------------------ */

void AItems::Dropped()
{
	SetActorEnableCollision(true);

	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ItemMesh->SetCollisionObjectType(ECC_WorldDynamic);
	ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	HoldingCharacter = nullptr;
	SetOwner(nullptr);
}