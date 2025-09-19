#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items.generated.h"

class APalletActor;
class AWarehouseCharacter;
class UCargoComponent;

UCLASS()
class WAREHOUSESIM_API AItems : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	AItems();

	// Engine overrides
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Core functions
	void Dropped();

	/** Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* ItemMesh;

	/** Overlap functions */
	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	/** First-person item placement */
	UPROPERTY(EditDefaultsOnly, Category = "Item|FirstPerson")
	FVector FirstPersonItemOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Item|FirstPerson")
	FRotator FirstPersonItemRotation = FRotator::ZeroRotator;

	/** Hand IK helpers */
	UFUNCTION(BlueprintCallable)
	FVector GetHandIKSocketLocation() const;

	UFUNCTION(BlueprintCallable)
	FVector GetLeftHandIKSocketLocation() const;

	/** Ownership */
	UPROPERTY(Replicated)
	AActor* HoldingCharacter = nullptr;

	bool IsHeld() const { return HoldingCharacter != nullptr; }

	/** Economy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Economy")
	int32 Value = 0;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Cargo")
	int32 ItemValue = 0;

	UFUNCTION()
	void OnRep_ItemValue();

	void SetItemValue(int32 NewValue);

	/** Pallet handling */
	UPROPERTY()
	int32 AssignedPalletSlotIndex = INDEX_NONE;

	UPROPERTY()
	APalletActor* PlacedOnPallet = nullptr;

	UPROPERTY(Replicated)
	bool bWasPlacedOnPallet = false;

	UPROPERTY()
	AWarehouseCharacter* DroppedByCharacter = nullptr;

	/** Categories */
	UPROPERTY(ReplicatedUsing = OnRep_Category)
	int32 Category;

	UFUNCTION()
	void SetCategory(int32 NewCategory);

	UFUNCTION(BlueprintImplementableEvent)
	void OnCategoryChanged(int32 NewCategory);

	UFUNCTION()
	void ApplyColorForCategory();

	UFUNCTION(Server, Reliable)
	void MulticastApplyColor();

protected:
	// Engine overrides
	virtual void BeginPlay() override;

	/** Replication */
	UFUNCTION()
	void OnRep_Category();

	/** Category materials */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category|Materials")
	UMaterialInterface* RedMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category|Materials")
	UMaterialInterface* BlueMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category|Materials")
	UMaterialInterface* GreenMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Category|Materials")
	UMaterialInterface* YellowMaterial;

public:
	/** Cargo access */
	UFUNCTION(BlueprintCallable)
	UCargoComponent* GetOwningCargoComponent() const;
};