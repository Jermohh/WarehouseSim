#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GarageDoorActor.generated.h"

class ADoorSirenBase;
class ATruckActor;

UCLASS()
class WAREHOUSESIM_API AGarageDoorActor : public AActor
{
	GENERATED_BODY()

public:
	AGarageDoorActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void CloseDoor();

	UFUNCTION(BlueprintCallable)
	void ResetTimer();

	UFUNCTION(Server, Reliable)
	void ServerBuyDoor(AWarehouseCharacter* Buyer);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_TriggerSirenColorChange();

	UPROPERTY(EditAnywhere, Category = "Truck")
	TSubclassOf<ATruckActor> TruckClass;

	UPROPERTY(EditAnywhere, Category = "Truck")
	FVector TruckSpawnLocation;

	UPROPERTY(EditAnywhere, Category = "Truck")
	FRotator TruckSpawnRotation = FRotator::ZeroRotator;

	UPROPERTY()
	ATruckActor* ActiveTruck = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Garage")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garage")
	float TimeUntilTruckArrives = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Garage")
	class UWidgetComponent* TimerWidget;

	UFUNCTION(BlueprintPure)
	float GetRemainingTruckTime() const;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime)
	float ReplicatedRemainingTime = 0.f;

	UFUNCTION()
	void OnRep_RemainingTime();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garage")
	ADoorSirenBase* DoorSirenRef;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	bool bDoorUnlocked = false;
	
	UFUNCTION()
	void OnRep_DoorOpened();

	UFUNCTION()
	void OpenDoor();

	UFUNCTION(BlueprintCallable, Category = "Garage")
	float GetRemainingArrivalTime() const { return GetRemainingTruckTime(); }

	UFUNCTION(BlueprintCallable, Category = "Garage")
	void StartArrivalIn(float Seconds);
protected:
	UPROPERTY(EditAnywhere)
	FTimerHandle DoorOpenTimer;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DoorOpened)
	bool bDoorOpened = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Garage")
	int32 DoorCost = 100;

public:
	FORCEINLINE bool IsDoorOpened() const { return bDoorOpened; }

};
