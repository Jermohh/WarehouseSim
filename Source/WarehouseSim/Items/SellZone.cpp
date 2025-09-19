#include "SellZone.h"
#include "Components/BoxComponent.h"
#include "WarehouseSim/Items/Items.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"


ASellZone::ASellZone()
{
	PrimaryActorTick.bCanEverTick = false;


	TriggerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerZone"));
	RootComponent = TriggerZone;


	TriggerZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerZone->SetCollisionObjectType(ECC_WorldStatic);
	TriggerZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerZone->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	TriggerZone->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	TriggerZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);


	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &ASellZone::OnOverlapBegin);
}


void ASellZone::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	AItems* Item = Cast<AItems>(OtherActor);
	if (!Item || !HasAuthority()) return;


	if (!ItemsInZone.Contains(Item))
	{
		ItemsInZone.Add(Item);
	}
}


void ASellZone::SellItems()
{
	if (!HasAuthority()) return;


	for (int32 i = ItemsInZone.Num() - 1; i >= 0; --i)
	{
		AItems* Item = ItemsInZone[i];
		if (!IsValid(Item))
		{
			ItemsInZone.RemoveAt(i);
			continue;
		}


		if (Item->Category != SellCategory)
		{
			UE_LOG(LogTemp, Warning, TEXT("SellZone: Item %s has category %d but zone is category %d"),
				*Item->GetName(), Item->Category, SellCategory);
			continue;
		}


		AWarehouseCharacter* Char = Item->DroppedByCharacter;
		if (Char)
		{
			Char->Money += Item->ItemValue;
			if (Char->IsLocallyControlled())
			{
				Char->UpdateHUDMoney();
			}
		}


		Item->Destroy();
		ItemsInZone.RemoveAt(i);
	}
}