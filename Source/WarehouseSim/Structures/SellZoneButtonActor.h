#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SellZoneButtonActor.generated.h"

class UBoxComponent;
class ASellZone;
class AGarageSellDoorActor;
class ASellingTruckActor;

UCLASS()
class WAREHOUSESIM_API ASellZoneButtonActor : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	ASellZoneButtonActor();

	/** Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* ButtonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* TriggerBox;

	/** Linked actors */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "SellZone")
	ASellZone* LinkedSellZone;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "SellZone")
	AGarageSellDoorActor* LinkedGarageDoor;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "SellZone")
	ASellingTruckActor* LinkedSellingTruckActor;

protected:
	// Engine overrides
	virtual void BeginPlay() override;

	// Overlap events
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
};