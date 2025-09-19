#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/BoxComponent.h"
#include "ConveyorBelt.generated.h"

UCLASS()
class WAREHOUSESIM_API AConveyorBelt : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	AConveyorBelt();

	// Conveyor speed
	static float ConveyorSpeedMultiplier;

	// Core
	UBoxComponent* GetClosestSnapBox(const FVector& HitLocation) const;

	// Components
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SnapPoint;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BeltMesh;

	// Snapping
	UPROPERTY(VisibleAnywhere, Category = "Snapping")
	UBoxComponent* SnapFront;

	UPROPERTY(VisibleAnywhere, Category = "Snapping")
	UBoxComponent* SnapBack;

	UPROPERTY(VisibleAnywhere, Category = "Snapping")
	UBoxComponent* SnapLeft;

	UPROPERTY(VisibleAnywhere, Category = "Snapping")
	UBoxComponent* SnapRight;

	UPROPERTY(VisibleAnywhere, Category = "Snapping")
	UBoxComponent* SnapFrontLeft;

	UPROPERTY(VisibleAnywhere, Category = "Snapping")
	UBoxComponent* SnapFrontRight;

	// Scanner mount
	UPROPERTY(VisibleAnywhere, Category = "Scanner")
	USceneComponent* ScannerMountCenter = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Scanner")
	UBoxComponent* ScannerSnapCenter = nullptr;

protected:
	// Engine overrides
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Conveyor properties
	UPROPERTY(EditAnywhere, Category = "Conveyor")
	float ConveyorSpeed = 100.f;

	UPROPERTY(EditAnywhere, Category = "Conveyor")
	FVector2D MeshScale = FVector2D(1.f, 1.f);

	UPROPERTY(EditAnywhere, Category = "Conveyor")
	float BeltLength = 400.f;

	UPROPERTY(EditAnywhere, Category = "Conveyor")
	float BeltWidth = 200.f;

	// Materials
	UPROPERTY(EditAnywhere)
	UMaterialInterface* ConveyorMaterial;

	// Runtime storage
	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshComponents;

	UPROPERTY()
	TArray<UBoxComponent*> MoveZones;

	UPROPERTY(BlueprintReadWrite)
	TMap<AActor*, float> BoxExitTimers;

	// Main move zone
	UPROPERTY(VisibleAnywhere, Category = "Conveyor")
	UBoxComponent* MoveZone;

	// Helpers
	void MoveBoxes(float DeltaTime);
};