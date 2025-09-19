#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CargoComponent.generated.h"

class AWarehouseCharacter;
class AScannerStation;
class AItems;
class APalletActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WAREHOUSESIM_API UCargoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCargoComponent();

	/** Item handling */
	void PickupItem(AItems* Item);
	void DropItem();
	bool IsHoldingItem() const { return HeldItem != nullptr; }

	/** Pallet handling */
	void PickupPallet(APalletActor* Pallet);
	void DropPallet();
	bool IsHoldingPallet() const { return HeldPallet != nullptr; }

	/** Pallet placement */
	void TryPlaceItemOnPallet();

	/** Scanner */
	void RequestItemScan(AItems* Item, AScannerStation* Scanner);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** References */
	UPROPERTY()
	AWarehouseCharacter* OwnerCharacter;

	/** Item data */
	UPROPERTY(Replicated) AItems* HeldItem = nullptr;
	AItems* LastHeldItem = nullptr;

	/** Pallet data */
	UPROPERTY(Replicated) APalletActor* HeldPallet = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Cargo|Pallet")
	FVector FirstPersonPalletOffset = FVector(50.f, 0.f, -20.f);

	UPROPERTY(EditDefaultsOnly, Category = "Cargo|Pallet")
	FRotator FirstPersonPalletRotation = FRotator::ZeroRotator;

	/** Networking - Items */
	UFUNCTION(Server, Reliable) void ServerPickupItem(AItems* Item);
	UFUNCTION(Server, Reliable) void ServerDropItem();
	UFUNCTION(NetMulticast, Reliable) void MulticastAttachHeldItem(AItems* ItemToAttach);
	UFUNCTION(NetMulticast, Reliable) void MulticastOnItemDropped(AItems* DroppedItem);
	UFUNCTION(Server, Reliable) void ServerPlaceItemOnPallet(APalletActor* TargetPallet);
	UFUNCTION(NetMulticast, Reliable) void MulticastPlaceItemOnPallet(APalletActor* TargetPallet, int32 SlotIndex);

	/** Networking - Pallets */
	UFUNCTION(Server, Reliable) void ServerPickupPallet(APalletActor* Pallet);
	UFUNCTION(NetMulticast, Reliable) void MulticastPickupPallet(APalletActor* Pallet);
	UFUNCTION(Server, Reliable) void ServerDropPallet();
	UFUNCTION(NetMulticast, Reliable) void MulticastDropPallet();

	/** Networking - Scanner */
	UFUNCTION(Server, Reliable) void Server_RequestScan(AItems* Item, AScannerStation* Scanner);

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Getters */
	FORCEINLINE AItems* GetHeldItem() const { return HeldItem; }
	FORCEINLINE APalletActor* GetHeldPallet() const { return HeldPallet; }
};
