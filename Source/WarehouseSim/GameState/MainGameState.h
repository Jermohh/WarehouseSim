#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MainGameState.generated.h"

class AConveyorBelt;
class AItems;
class AScannerStation;
class APalletActor;

UCLASS()
class WAREHOUSESIM_API AMainGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMainGameState();

	/* ---------------------------- Conveyor Upgrade ---------------------------- */
	UPROPERTY(ReplicatedUsing = OnRep_ConveyorSpeedMultiplier, BlueprintReadOnly, Category = "Upgrades")
	float ConveyorSpeedMultiplier = 1.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Upgrades")
	int32 ConveyorUpgradeLevel = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrades")
	int32 BaseConveyorUpgradeCost = 1;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Upgrades")
	int32 ConveyorUpgradeCost = 0;

	UFUNCTION(BlueprintCallable, Category = "Upgrades")
	int32 GetConveyorUpgradeLevel() const { return ConveyorUpgradeLevel; }

	void UpgradeConveyorSpeed();

	UPROPERTY(EditDefaultsOnly, Category = "SaveLoad")
	TSubclassOf<AConveyorBelt> ConveyorClass;

	/* ---------------------------- Items (Save/Load) ---------------------------- */
	UPROPERTY(EditDefaultsOnly, Category = "SaveLoad")
	TSubclassOf<AItems> ItemClass;

	/* ---------------------------- Truck Box Spawner ---------------------------- */
	UPROPERTY(ReplicatedUsing = OnRep_TruckUpgradeChanged, BlueprintReadOnly, Category = "Upgrades")
	int32 TruckUpgradeLevel = 0;

	UPROPERTY(ReplicatedUsing = OnRep_TruckUpgradeChanged, BlueprintReadOnly, Category = "Upgrades")
	int32 TruckUpgradeCost = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrades")
	int32 BaseTruckUpgradeCost = 100;

	UFUNCTION() void OnRep_TruckUpgradeChanged();
	UFUNCTION(BlueprintCallable, Category = "Upgrades") void UpgradeTruckBoxSpawn();

	/* ---------------------------- Truck Spawn Time ---------------------------- */
	UPROPERTY(ReplicatedUsing = OnRep_TruckSpawnTimeMultiplier, BlueprintReadOnly, Category = "Upgrades|TruckSpawn")
	float TruckSpawnTimeMultiplier = 1.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Upgrades|TruckSpawn")
	int32 TruckSpawnTimeUpgradeLevel = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Upgrades|TruckSpawn")
	int32 TruckSpawnTimeUpgradeCost = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrades|TruckSpawn")
	int32 BaseTruckSpawnTimeUpgradeCost = 150;

	UFUNCTION(BlueprintCallable, Category = "Upgrades|TruckSpawn", meta = (BlueprintAuthorityOnly = "true"))
	void UpgradeTruckSpawnTime();

	UFUNCTION() void OnRep_TruckSpawnTimeMultiplier();

	/* ---------------------------- Scanner Value ---------------------------- */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Upgrades|Scanner")
	int32 ScannerValueUpgradeLevel = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Upgrades|Scanner")
	int32 ScannerValueUpgradeCost = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrades|Scanner")
	int32 BaseScannerValueUpgradeCost = 200;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Upgrades|Scanner")
	float ScannerBonusMinPct = 0.f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Upgrades|Scanner")
	float ScannerBonusMaxPct = 0.f;

	UFUNCTION(BlueprintCallable, Category = "Upgrades|Scanner", meta = (BlueprintAuthorityOnly = "true"))
	void UpgradeScannerValue();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrades")
	int32 ScannerStationCost = 100;

	UPROPERTY(EditDefaultsOnly, Category = "SaveLoad")
	TSubclassOf<AScannerStation> ScannerClass;

	/* ---------------------------- Store / Costs ---------------------------- */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrades")
	int32 ConveyorBeltCost = 100;

	/* ---------------------------- Pallets ---------------------------- */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SaveLoad")
	TSubclassOf<APalletActor> PalletClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Store|Costs")
	int32 PalletCost = 150;

	/* ---------------------------- Save / Load ---------------------------- */
	UFUNCTION(BlueprintCallable, Category = "SaveLoad") void SaveGameData();
	UFUNCTION(BlueprintCallable, Category = "SaveLoad") void LoadGameData();

	UPROPERTY() bool bHasLoadedSave = false;

	UFUNCTION(BlueprintCallable, Category = "SaveLoad") void ClearSave();

protected:
	UFUNCTION() void OnRep_ConveyorSpeedMultiplier();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
