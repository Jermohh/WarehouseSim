#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GarageSellDoorActor.generated.h"

class UStaticMeshComponent;
class ASellZone;
class ASellingTruckActor;

UCLASS()
class WAREHOUSESIM_API AGarageSellDoorActor : public AActor
{
	GENERATED_BODY()

public:
	AGarageSellDoorActor();
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Garage")
	UStaticMeshComponent* DoorMesh;

	UFUNCTION(BlueprintCallable)
	void ActivateSell();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Garage")
	ASellZone* LinkedSellZone;

	UPROPERTY(Replicated)
	FVector TargetDoorLocation;

	UPROPERTY(EditAnywhere, Category = "Door")
	float DoorInterpSpeed = 250.f;

	UFUNCTION()
	void OnRep_ShouldCloseDoor();

	UPROPERTY(ReplicatedUsing = OnRep_ShouldCloseDoor)
	bool bShouldCloseDoor = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Garage Door")
	void PlayCloseDoorTimeline();

	UFUNCTION(BlueprintImplementableEvent, Category = "Garage Door")
	void PlayOpenDoorTimeline();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayCloseDoorTimeline();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayOpenDoorTimeline();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void CloseDoor();

	UFUNCTION()
	void OpenDoor();

	FVector InitialDoorLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float DoorMoveDistance = 430.f;

	FTimerHandle DoorTimerHandle;
};
