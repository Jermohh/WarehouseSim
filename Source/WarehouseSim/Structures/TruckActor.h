#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TruckActor.generated.h"

class AItems;

UCLASS()
class WAREHOUSESIM_API ATruckActor : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	ATruckActor();

	/** Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* TruckMesh;

	/** Truck settings */
	UPROPERTY(EditAnywhere, Category = "Truck")
	int32 BoxesToDeliver = 5;

	UPROPERTY(EditAnywhere, Category = "Truck")
	TSubclassOf<AItems> BoxClass;

	// Actions
	UFUNCTION()
	void ArriveAndUnload(int32 ExtraBoxes = 0);

	UFUNCTION()
	void SetExtraBoxes(int32 ExtraBoxes);

protected:
	// Engine overrides
	virtual void BeginPlay() override;

	// Box spawning
	UPROPERTY(EditAnywhere, Category = "Truck|Boxes")
	FVector BoxSpawnOffset = FVector(0.f, -600.f, 390.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Truck|Boxes")
	float BoxSpawnRadius = 100.f;

	UPROPERTY(EditAnywhere, Category = "Truck|Boxes")
	int32 BoxesPerRow = 4;

	UPROPERTY(EditAnywhere, Category = "Truck|Boxes")
	float BoxSpacingX = 80.f;

	UPROPERTY(EditAnywhere, Category = "Truck|Boxes")
	float BoxSpacingY = 80.f;

	void SpawnBoxes(int32 ExtraBoxes = 0);
};