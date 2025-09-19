#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WarehouseSim/Structures/ConveyorBelt.h"
#include "WarehouseCharacter.generated.h"

class UUpgradeMenu;
class ADoorButtonActor;
class AItems;
class ASellZoneButtonActor;
class AGarageDoorActor;
class ABuyComputer;
class AScannerStation;
class APalletActor;

UCLASS()
class WAREHOUSESIM_API AWarehouseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AWarehouseCharacter();

	/** Handles HUD money updates */
	void UpdateHUDMoney();

	/** References */
	UPROPERTY()
	ADoorButtonActor* OverlappingDoorButton = nullptr;

	UPROPERTY()
	ASellZoneButtonActor* OverlappingSellZoneButton = nullptr;

	/** Cargo handling */
	UPROPERTY(VisibleAnywhere, Category = "Cargo")
	USceneComponent* HandAttachPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCargoComponent* CargoComponent;

	/** Currency */
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Money)
	int32 Money = 0;

	/** Upgrades */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	int32 TruckUpgradeLevel = 0;

	UPROPERTY(Replicated)
	bool bInRangeOfComputer = false;

	UFUNCTION(BlueprintCallable) void ApplyConveyorUpgrade();
	UFUNCTION(Server, Reliable) void Server_RequestConveyorUpgrade();

	UFUNCTION(BlueprintCallable) void ApplyTruckUpgrade();
	UFUNCTION(Server, Reliable) void Server_RequestTruckUpgrade();

	UFUNCTION(BlueprintCallable, Category = "Upgrades") void ApplyTruckSpawnTimeUpgrade();
	UFUNCTION(Server, Reliable) void Server_RequestTruckSpawnTimeUpgrade();

	UFUNCTION(BlueprintCallable, Category = "Upgrades") void ApplyScannerValueUpgrade();
	UFUNCTION(Server, Reliable) void Server_RequestScannerValueUpgrade();

	/** Conveyor handling */
	UPROPERTY(ReplicatedUsing = OnRep_HeldBelt)
	AConveyorBelt* HeldBelt = nullptr;

	UFUNCTION() void OnRep_HeldBelt();
	UFUNCTION(Server, Reliable) void ServerPickupConveyor(AConveyorBelt* Belt);
	UFUNCTION(Server, Reliable) void ServerDropConveyor(const FVector& DropLocation);
	UFUNCTION(Server, Reliable) void ServerSnapPlaceConveyor(AConveyorBelt* Belt, const FVector& Location, const FRotator& Rotation);
	UFUNCTION(NetMulticast, Reliable) void MulticastAttachConveyor(AConveyorBelt* Belt);
	UFUNCTION(NetMulticast, Reliable) void MulticastDetachConveyor(AConveyorBelt* Belt);
	UFUNCTION() void OnEditConveyorPressed();

	/** Scanner handling */
	UPROPERTY(ReplicatedUsing = OnRep_HeldScanner)
	AScannerStation* HeldScanner = nullptr;

	UFUNCTION() void OnRep_HeldScanner();
	UFUNCTION(Server, Reliable) void ServerPickupScanner(AScannerStation* Scanner);
	UFUNCTION(Server, Reliable) void ServerDropScanner(const FVector& DropLocation);
	UFUNCTION(Server, Reliable) void ServerAttachScannerCenter(AScannerStation* Scanner, AConveyorBelt* Belt);
	UFUNCTION() void OnEditScannerPressed();

	/** Purchases */
	UFUNCTION(BlueprintCallable, Category = "BuyObjects") void BuyConveyorBelt();
	UFUNCTION(Server, Reliable) void Server_BuyConveyorBelt();

	UFUNCTION(BlueprintCallable, Category = "BuyObjects") void BuyScannerStation();
	UFUNCTION(Server, Reliable) void Server_BuyScannerStation();

	UPROPERTY(EditDefaultsOnly, Category = "Buildables|Scanner")
	TSubclassOf<AScannerStation> ScannerStationClass;

	UPROPERTY(EditDefaultsOnly, Category = "BuyObjects")
	TSubclassOf<AConveyorBelt> ConveyorBeltClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuyObjects")
	FVector ConveyorSpawnLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuyObjects")
	FRotator ConveyorSpawnRotation = FRotator::ZeroRotator;

	UFUNCTION(BlueprintCallable, Category = "BuyObjects") void BuyPallet();
	UFUNCTION(Server, Reliable) void Server_BuyPallet();

	UPROPERTY(EditDefaultsOnly, Category = "Buildables|Pallet")
	TSubclassOf<APalletActor> PalletClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuyObjects")
	FVector PalletSpawnLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuyObjects")
	FRotator PalletSpawnRotation = FRotator::ZeroRotator;

	/** Save & Load */
	void HandleSaveGame();
	void HandleLoadGame();
	UFUNCTION() void HandleClearSave();

	/** Selling */
	UFUNCTION(Server, Reliable) void ServerRequestSellZoneSell(class ASellZone* SellZone);
	UFUNCTION() void OnSellZoneButtonPressed();
	UFUNCTION(Server, Reliable) void ServerActivateGarageDoor(class AGarageSellDoorActor* Door);
	UFUNCTION(Server, Reliable) void ServerActivateSellingTruck(class ASellingTruckActor* Truck);

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Movement */
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	/** Input Handlers */
	UFUNCTION() void OnPickupPressed();
	UFUNCTION() void OnHeavyObjectPickupPressed();
	UFUNCTION() void OnBuyDoorPressed();
	UFUNCTION() void OnPlaceItemPressed();

	/** Networking */
	UFUNCTION(Server, Reliable) void ServerBuyGarageDoor(AGarageDoorActor* Door);
	UFUNCTION(Server, Reliable) void ServerSetWalkSpeed(float NewSpeed);

	/** Replication callbacks */
	UFUNCTION() void OnRep_Money();

	/** Door handling */
	void HandleDoorClose();

	UFUNCTION(Server, Reliable)
	void Server_RequestDoorClose(ADoorButtonActor* Button);

	/** UI */
	void ToggleUpgradeMenu();
	UPROPERTY(EditAnywhere, Category = "UI") TSubclassOf<UUpgradeMenu> UpgradeMenuClass;
	UPROPERTY() UUpgradeMenu* UpgradeMenuInstance = nullptr;
	bool bIsUpgradeMenuOpen = false;

	/** Belt offsets */
	UPROPERTY(EditDefaultsOnly, Category = "Held Belt Offset") FVector BeltHoldOffset = FVector(50.f, 20.f, -20.f);
	UPROPERTY(EditDefaultsOnly, Category = "Held Belt Offset") FRotator BeltHoldRotation = FRotator::ZeroRotator;

	/** Camera & Mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera") class UCameraComponent* FirstPersonCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh") class USkeletalMeshComponent* FirstPersonMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh") class USkeletalMeshComponent* ThirdPersonMesh;

	/** Helpers */
	void TraceUnderCrosshair(FHitResult& TraceHitResult);

public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }
	FORCEINLINE USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; }
};
