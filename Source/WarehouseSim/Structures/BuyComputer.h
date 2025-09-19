#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuyComputer.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class WAREHOUSESIM_API ABuyComputer : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	ABuyComputer();

protected:
	// Engine overrides
	virtual void BeginPlay() override;

	/** Components */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ComputerMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* OverlapBox;

	/** Overlap events */
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
};