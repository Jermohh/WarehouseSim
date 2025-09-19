#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SellingTruckActor.generated.h"

UCLASS()
class WAREHOUSESIM_API ASellingTruckActor : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	ASellingTruckActor();

	/** Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TruckMesh;

	/** Actions */
	UFUNCTION()
	void ActivateTruck();

	UFUNCTION()
	void TriggerReturnTruck();

	/** RPCs */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_TriggerSellingTruck();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReturnTruck();

	/** Replication */
	UPROPERTY(ReplicatedUsing = OnRep_TruckTriggered)
	bool bTruckTriggered = false;

	UFUNCTION()
	void OnRep_TruckTriggered();

	// Blueprint events
	UFUNCTION(BlueprintImplementableEvent, Category = "Garage Truck")
	void DriveTruckAway();

	UFUNCTION(BlueprintImplementableEvent, Category = "Garage Truck")
	void ReturnTruck();

	// Timers
	void HandleReturnTruck();
	FTimerHandle ReturnTruckTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Garage Truck")
	float ReturnDelay = 10.f;

protected:
	// Engine overrides
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};