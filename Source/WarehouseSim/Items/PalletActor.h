#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PalletActor.generated.h"

UCLASS()
class WAREHOUSESIM_API APalletActor : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	APalletActor();

	// Slot state updates
	UFUNCTION()
	void MarkSlotOccupied(int32 Index);

	UFUNCTION()
	void MarkSlotEmpty(int32 Index);

	// Slot accessors
	UFUNCTION(BlueprintCallable)
	USceneComponent* GetNextAvailableSlot();

	UFUNCTION(BlueprintCallable)
	void SetSlotOccupiedArray(const TArray<bool>& In);

	// Mesh & IK helpers
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent* PalletMesh;

	UFUNCTION(BlueprintCallable)
	FVector GetHandIKSocketLocation() const;

	UFUNCTION(BlueprintCallable)
	FVector GetLeftHandIKSocketLocation() const;

	// Slot layout
	UPROPERTY(VisibleAnywhere)
	TArray<USceneComponent*> ItemSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pallet")
	int32 TotalSlots = 4;

protected:
	// Storage root for slots
	UPROPERTY(VisibleAnywhere)
	USceneComponent* StorageRoot;

	// Occupancy flags for slots
	UPROPERTY()
	TArray<bool> SlotOccupied;

public:
	// Inline accessors
	FORCEINLINE USceneComponent* GetStorageRoot() const { return StorageRoot; }
	FORCEINLINE const TArray<bool>& GetSlotOccupied() const { return SlotOccupied; }
};