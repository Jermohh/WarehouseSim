#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScannerStation.generated.h"

class USceneComponent;
class UCameraComponent;
class UBoxComponent;
class AItems;

UCLASS()
class WAREHOUSESIM_API AScannerStation : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	AScannerStation();

	// Visuals / root (declared here; usage is in .cpp via ScannerTrigger as the actual root)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scanner")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scanner")
	UStaticMeshComponent* Mesh;

	// Scanning
	void AssignScannedValue(AItems* Item);

	UFUNCTION(Server, Reliable)
	void Server_RequestScan(AItems* Item);

	// Mounting & pickup/drop
	UFUNCTION(Server, Reliable)
	void Server_AttachToMount(USceneComponent* Mount);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnPickedUp(UCameraComponent* AttachCamera);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDropped();

protected:
	// Engine
	virtual void BeginPlay() override;

	// Trigger volume for scan detection
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* ScannerTrigger;

	// Overlap handler
	UFUNCTION()
	void OnScannerOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};