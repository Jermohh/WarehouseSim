#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WarehouseSaveGame.generated.h"

/** Scanner station placement */
USTRUCT(BlueprintType)
struct FScannerSaveData
{
	GENERATED_BODY()

	UPROPERTY() FVector  Location;
	UPROPERTY() FRotator Rotation;
};

/** Conveyor placement */
USTRUCT(BlueprintType)
struct FConveyorSaveData
{
	GENERATED_BODY()

	UPROPERTY() FVector  Location;
	UPROPERTY() FRotator Rotation;
};

/** Loose item state */
USTRUCT(BlueprintType)
struct FItemSaveData
{
	GENERATED_BODY()

	UPROPERTY() FVector  Location;
	UPROPERTY() FRotator Rotation;

	UPROPERTY() int32 ItemValue = 0;
	UPROPERTY() int32 Category = 0;
};

/** Garage door snapshot */
USTRUCT(BlueprintType)
struct FGarageDoorSaveData
{
	GENERATED_BODY()

	UPROPERTY() FName DoorName;                 // stable identity (GetFName)
	UPROPERTY() bool  bUnlocked = false; // bDoorUnlocked
	UPROPERTY() bool  bIsOpen = false; // current open state
	UPROPERTY() float RemainingArrivalTime = 0.f;  // seconds until next auto-open
};

/** Pallet snapshot */
USTRUCT(BlueprintType)
struct FPalletSaveData
{
	GENERATED_BODY()

	UPROPERTY() FVector  Location;
	UPROPERTY() FRotator Rotation;

	UPROPERTY() int32        TotalSlots = 0;
	UPROPERTY() TArray<bool> SlotOccupied;
};

UCLASS()
class WAREHOUSESIM_API UWarehouseSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Upgrade levels / costs snapshot
	UPROPERTY(VisibleAnywhere) int32 ConveyorUpgradeLevel = 0;
	UPROPERTY(VisibleAnywhere) int32 TruckUpgradeLevel = 0;
	UPROPERTY(VisibleAnywhere) int32 TruckSpawnTimeUpgradeLevel = 0;
	UPROPERTY(VisibleAnywhere) int32 ScannerValueUpgradeLevel = 0;

	// Player money snapshot
	UPROPERTY(VisibleAnywhere) int32 SavedPlayerMoney = 0;

	// World objects
	UPROPERTY(VisibleAnywhere) TArray<FConveyorSaveData>     SavedConveyors;
	UPROPERTY(VisibleAnywhere) TArray<FName>                 UnlockedDoors;
	UPROPERTY(VisibleAnywhere) TArray<FItemSaveData>         SavedItems;
	UPROPERTY(VisibleAnywhere) TArray<FScannerSaveData>      SavedScanners;
	UPROPERTY(VisibleAnywhere) TArray<FGarageDoorSaveData>   SavedGarageDoors;
	UPROPERTY(VisibleAnywhere) TArray<FPalletSaveData>       SavedPallets;
};
