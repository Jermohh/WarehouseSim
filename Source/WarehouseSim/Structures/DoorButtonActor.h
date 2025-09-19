#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "DoorButtonActor.generated.h"

class AGarageDoorActor;

UCLASS()
class WAREHOUSESIM_API ADoorButtonActor : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	ADoorButtonActor();

	// Engine overrides
	virtual void BeginPlay() override;

	// Door interaction
	UFUNCTION(BlueprintCallable)
	void RequestDoorClose();

protected:
	// Overlap events
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

public:
	// Components
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ButtonMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxSphere;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* HoverWidget;

	// Linked door
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AGarageDoorActor* LinkedGarageDoor;
};