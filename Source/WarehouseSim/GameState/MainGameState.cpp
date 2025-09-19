#include "MainGameState.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

#include "WarehouseSim/Structures/ConveyorBelt.h"
#include "WarehouseSim/Structures/TruckActor.h"
#include "WarehouseSim/Structures/GarageDoorActor.h"

#include "WarehouseSim/GameState/WarehouseSaveGame.h"
#include "WarehouseSim/GameState/MainPlayerState.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"

#include "WarehouseSim/Items/Items.h"
#include "WarehouseSim/Items/ScannerStation.h"
#include "WarehouseSim/Items/PalletActor.h"

AMainGameState::AMainGameState()
{
	bReplicates = true;
}

void AMainGameState::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		LoadGameData();
	}
}

/* ---------------------------- Conveyor Upgrade ---------------------------- */

void AMainGameState::UpgradeConveyorSpeed()
{
	ConveyorUpgradeLevel++;
	ConveyorSpeedMultiplier = 1.f + ConveyorUpgradeLevel * 0.25f;
	ConveyorUpgradeCost = BaseConveyorUpgradeCost + (ConveyorUpgradeLevel * 25);

	OnRep_ConveyorSpeedMultiplier();
}

void AMainGameState::OnRep_ConveyorSpeedMultiplier()
{
	AConveyorBelt::ConveyorSpeedMultiplier = ConveyorSpeedMultiplier;
}

/* ---------------------------- Clear Save ---------------------------- */

void AMainGameState::ClearSave()
{
	UGameplayStatics::DeleteGameInSlot(TEXT("WarehouseSaveSlot"), 0);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("Save file deleted."));
	}
}

/* ---------------------------- Truck Box Spawner ---------------------------- */

void AMainGameState::UpgradeTruckBoxSpawn()
{
	TruckUpgradeLevel++;
	TruckUpgradeCost = BaseTruckUpgradeCost + (TruckUpgradeLevel * 50);

	OnRep_TruckUpgradeChanged();

	UE_LOG(LogTemp, Warning, TEXT("[GameState] Truck Upgrade applied. New Level: %d, New Cost: %d"),
		TruckUpgradeLevel, TruckUpgradeCost);
}

void AMainGameState::OnRep_TruckUpgradeChanged()
{
	UE_LOG(LogTemp, Warning, TEXT("[CLIENT] Truck Upgrade Updated: Level %d, Cost %d"),
		TruckUpgradeLevel, TruckUpgradeCost);
}

/* ---------------------------- Truck Spawn Time ---------------------------- */

void AMainGameState::UpgradeTruckSpawnTime()
{
	if (!HasAuthority()) return;

	TruckSpawnTimeUpgradeLevel++;
	TruckSpawnTimeUpgradeCost = BaseTruckSpawnTimeUpgradeCost + (TruckSpawnTimeUpgradeLevel * 50);

	for (TActorIterator<AGarageDoorActor> It(GetWorld()); It; ++It)
	{
		if (AGarageDoorActor* Door = *It)
		{
			Door->TimeUntilTruckArrives *= 0.95f;
			Door->ResetTimer();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[GameState] Truck SpawnTime upgraded: Level %d, NextCost %d"),
		TruckSpawnTimeUpgradeLevel, TruckSpawnTimeUpgradeCost);
}

void AMainGameState::OnRep_TruckSpawnTimeMultiplier()
{
	// Intentionally empty (kept for replication notifications)
}

/* ---------------------------- Scanner Value ---------------------------- */

void AMainGameState::UpgradeScannerValue()
{
	if (!HasAuthority()) return;

	ScannerValueUpgradeLevel++;
	ScannerValueUpgradeCost = BaseScannerValueUpgradeCost + (ScannerValueUpgradeLevel * 50);

	ScannerBonusMinPct = FMath::Clamp(ScannerBonusMinPct + 0.05f, 0.f, 1.0f);
	ScannerBonusMaxPct = FMath::Clamp(ScannerBonusMaxPct + 0.10f, ScannerBonusMinPct, 1.0f);

	UE_LOG(LogTemp, Warning, TEXT("[GameState] Scanner upgrade L:%d  Min:+%.0f%%  Max:+%.0f%%  NextCost:%d"),
		ScannerValueUpgradeLevel,
		ScannerBonusMinPct * 100.f,
		ScannerBonusMaxPct * 100.f,
		ScannerValueUpgradeCost);
}

/* ---------------------------- Replication ---------------------------- */

void AMainGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainGameState, ConveyorSpeedMultiplier);
	DOREPLIFETIME(AMainGameState, ConveyorUpgradeLevel);
	DOREPLIFETIME(AMainGameState, ConveyorUpgradeCost);

	DOREPLIFETIME(AMainGameState, TruckUpgradeLevel);
	DOREPLIFETIME(AMainGameState, TruckUpgradeCost);

	DOREPLIFETIME(AMainGameState, TruckSpawnTimeMultiplier);
	DOREPLIFETIME(AMainGameState, TruckSpawnTimeUpgradeLevel);
	DOREPLIFETIME(AMainGameState, TruckSpawnTimeUpgradeCost);

	DOREPLIFETIME(AMainGameState, ScannerValueUpgradeLevel);
	DOREPLIFETIME(AMainGameState, ScannerValueUpgradeCost);
	DOREPLIFETIME(AMainGameState, ScannerBonusMinPct);
	DOREPLIFETIME(AMainGameState, ScannerBonusMaxPct);
}

/* ---------------------------- Save / Load ---------------------------- */

void AMainGameState::SaveGameData()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
			*FString::Printf(TEXT("SaveGameData() entered. GS HasAuthority=%d"), HasAuthority() ? 1 : 0));
	}

	if (!HasAuthority())
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("No authority -> abort"));
		return;
	}

	UWarehouseSaveGame* SaveGameInstance =
		Cast<UWarehouseSaveGame>(UGameplayStatics::CreateSaveGameObject(UWarehouseSaveGame::StaticClass()));

	// Upgrades
	SaveGameInstance->ConveyorUpgradeLevel = ConveyorUpgradeLevel;
	SaveGameInstance->TruckUpgradeLevel = TruckUpgradeLevel;
	SaveGameInstance->TruckSpawnTimeUpgradeLevel = TruckSpawnTimeUpgradeLevel;
	SaveGameInstance->ScannerValueUpgradeLevel = ScannerValueUpgradeLevel;

	// Conveyors
	SaveGameInstance->SavedConveyors.Empty();
	for (TActorIterator<AConveyorBelt> It(GetWorld()); It; ++It)
	{
		if (AConveyorBelt* Belt = *It)
		{
			FConveyorSaveData Data;
			Data.Location = Belt->GetActorLocation();
			Data.Rotation = Belt->GetActorRotation();
			SaveGameInstance->SavedConveyors.Add(Data);
		}
	}

	// Player money
	for (APlayerState* PS : PlayerArray)
	{
		if (AMainPlayerState* MainPS = Cast<AMainPlayerState>(PS))
		{
			SaveGameInstance->SavedPlayerMoney = MainPS->PlayerMoney;
			break;
		}
	}

	// Items (skip held)
	SaveGameInstance->SavedItems.Empty();
	for (TActorIterator<AItems> It(GetWorld()); It; ++It)
	{
		AItems* Item = *It;
		if (!Item || Item->IsHeld()) continue;

		FItemSaveData ItemData;
		ItemData.Location = Item->GetActorLocation();
		ItemData.Rotation = Item->GetActorRotation();
		ItemData.ItemValue = Item->ItemValue;
		ItemData.Category = Item->Category;
		SaveGameInstance->SavedItems.Add(ItemData);
	}

	// Scanners
	SaveGameInstance->SavedScanners.Empty();
	for (TActorIterator<AScannerStation> It(GetWorld()); It; ++It)
	{
		if (AScannerStation* Scanner = *It)
		{
			FScannerSaveData Data;
			Data.Location = Scanner->GetActorLocation();
			Data.Rotation = Scanner->GetActorRotation();
			SaveGameInstance->SavedScanners.Add(Data);
		}
	}

	// Garage doors
	SaveGameInstance->SavedGarageDoors.Empty();
	for (TActorIterator<AGarageDoorActor> It(GetWorld()); It; ++It)
	{
		AGarageDoorActor* Door = *It;
		if (!Door) continue;

		FGarageDoorSaveData Row;
		Row.DoorName = Door->GetFName();
		Row.bUnlocked = Door->bDoorUnlocked;
		Row.bIsOpen = Door->IsDoorOpened();
		Row.RemainingArrivalTime = Door->GetRemainingArrivalTime();

		SaveGameInstance->SavedGarageDoors.Add(Row);
	}

	// Pallets
	SaveGameInstance->SavedPallets.Empty();
	for (TActorIterator<APalletActor> It(GetWorld()); It; ++It)
	{
		if (APalletActor* Pallet = *It)
		{
			FPalletSaveData Row;
			Row.Location = Pallet->GetActorLocation();
			Row.Rotation = Pallet->GetActorRotation();
			Row.TotalSlots = Pallet->TotalSlots;
			Row.SlotOccupied = Pallet->GetSlotOccupied();
			SaveGameInstance->SavedPallets.Add(Row);
		}
	}

	const int32 NumDoorsSaved = SaveGameInstance->SavedGarageDoors.Num();
	UE_LOG(LogTemp, Warning, TEXT("[SAVE] Doors saved: %d"), NumDoorsSaved);
	for (int32 i = 0; i < NumDoorsSaved; ++i)
	{
		const FGarageDoorSaveData& D = SaveGameInstance->SavedGarageDoors[i];
		UE_LOG(LogTemp, Warning, TEXT("[SAVE] Door[%d] Name=%s  Unlocked=%d  IsOpen=%d  RemArrival=%.2fs"),
			i, *D.DoorName.ToString(), D.bUnlocked ? 1 : 0, D.bIsOpen ? 1 : 0, D.RemainingArrivalTime);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green,
			*FString::Printf(TEXT("[SAVE] Doors:%d  Items:%d  Scanners:%d"),
				SaveGameInstance->SavedGarageDoors.Num(),
				SaveGameInstance->SavedItems.Num(),
				SaveGameInstance->SavedScanners.Num()));
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("WarehouseSaveSlot"), 0);
	UE_LOG(LogTemp, Warning, TEXT("[Save] Game saved to WarehouseSaveSlot"));

	const bool bOK = UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("WarehouseSaveSlot"), 0);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, bOK ? FColor::Green : FColor::Red,
		bOK ? TEXT("Saved to WarehouseSaveSlot") : TEXT("FAILED to save"));

	const bool bExists = UGameplayStatics::DoesSaveGameExist(TEXT("WarehouseSaveSlot"), 0);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, bExists ? FColor::Green : FColor::Red,
		bExists ? TEXT("Save file exists!") : TEXT("Save file NOT found!"));
}

void AMainGameState::LoadGameData()
{
	if (!HasAuthority()) return;
	if (!UGameplayStatics::DoesSaveGameExist(TEXT("WarehouseSaveSlot"), 0)) return;

	if (UWarehouseSaveGame* LoadGameInstance =
		Cast<UWarehouseSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("WarehouseSaveSlot"), 0)))
	{
		// Levels
		ConveyorUpgradeLevel = LoadGameInstance->ConveyorUpgradeLevel;
		TruckUpgradeLevel = LoadGameInstance->TruckUpgradeLevel;
		TruckSpawnTimeUpgradeLevel = LoadGameInstance->TruckSpawnTimeUpgradeLevel;
		ScannerValueUpgradeLevel = LoadGameInstance->ScannerValueUpgradeLevel;

		// Costs
		ConveyorSpeedMultiplier = 1.f + ConveyorUpgradeLevel * 0.25f;
		ConveyorUpgradeCost = BaseConveyorUpgradeCost + (ConveyorUpgradeLevel * 25);
		TruckUpgradeCost = BaseTruckUpgradeCost + (TruckUpgradeLevel * 50);
		TruckSpawnTimeUpgradeCost = BaseTruckSpawnTimeUpgradeCost + (TruckSpawnTimeUpgradeLevel * 50);
		ScannerValueUpgradeCost = BaseScannerValueUpgradeCost + (ScannerValueUpgradeLevel * 50);

		OnRep_ConveyorSpeedMultiplier();
		OnRep_TruckUpgradeChanged();

		// Player money
		for (APlayerState* PS : PlayerArray)
		{
			if (AMainPlayerState* MainPS = Cast<AMainPlayerState>(PS))
			{
				MainPS->PlayerMoney = LoadGameInstance->SavedPlayerMoney;
				break;
			}
		}

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				if (AWarehouseCharacter* Char = Cast<AWarehouseCharacter>(PC->GetPawn()))
				{
					if (AMainPlayerState* PS = Char->GetPlayerState<AMainPlayerState>())
					{
						Char->Money = PS->PlayerMoney;
						Char->UpdateHUDMoney();
					}
				}
			}
		}

		// Destroy & respawn conveyors
		for (TActorIterator<AConveyorBelt> It(GetWorld()); It; ++It) { It->Destroy(); }
		UClass* BeltClass = ConveyorClass ? *ConveyorClass : AConveyorBelt::StaticClass();
		for (const FConveyorSaveData& Data : LoadGameInstance->SavedConveyors)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			if (!GetWorld()->SpawnActor<AConveyorBelt>(BeltClass, Data.Location, Data.Rotation, Params))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn conveyor at %s"), *Data.Location.ToString());
			}
		}

		// Destroy & respawn items
		for (TActorIterator<AItems> It(GetWorld()); It; ++It) { if (AItems* E = *It) E->Destroy(); }
		UClass* ResolvedItemClass = ItemClass ? *ItemClass : AItems::StaticClass();
		for (const FItemSaveData& Data : LoadGameInstance->SavedItems)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AItems* NewItem = GetWorld()->SpawnActor<AItems>(ResolvedItemClass, Data.Location, Data.Rotation, Params);
			if (NewItem)
			{
				NewItem->SetItemValue(Data.ItemValue);
				NewItem->SetCategory(Data.Category);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn item at %s"), *Data.Location.ToString());
			}

			bHasLoadedSave = true;
			UE_LOG(LogTemp, Warning, TEXT("[Load] Game loaded from WarehouseSaveSlot"));
		}

		// Destroy & respawn scanners
		for (TActorIterator<AScannerStation> It(GetWorld()); It; ++It) { if (AScannerStation* E = *It) E->Destroy(); }
		UClass* ResolvedScannerClass = ScannerClass ? *ScannerClass : AScannerStation::StaticClass();
		for (const FScannerSaveData& Data : LoadGameInstance->SavedScanners)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			if (!GetWorld()->SpawnActor<AScannerStation>(ResolvedScannerClass, Data.Location, Data.Rotation, Params))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn scanner at %s"), *Data.Location.ToString());
			}
		}

		// Apply door state
		for (TActorIterator<AGarageDoorActor> It(GetWorld()); It; ++It)
		{
			AGarageDoorActor* Door = *It;
			if (!Door) continue;

			const FGarageDoorSaveData* Found =
				LoadGameInstance->SavedGarageDoors.FindByPredicate(
					[Door](const FGarageDoorSaveData& D) { return D.DoorName == Door->GetFName(); });

			if (!Found) continue;

			Door->bDoorUnlocked = Found->bUnlocked;

			if (Found->bIsOpen)
			{
				Door->OpenDoor();
			}
			else
			{
				Door->CloseDoor();
				if (Door->bDoorUnlocked && Found->RemainingArrivalTime > 0.f)
				{
					Door->StartArrivalIn(Found->RemainingArrivalTime);
				}
			}
		}

		// Destroy & respawn pallets
		for (TActorIterator<APalletActor> It(GetWorld()); It; ++It) { if (APalletActor* E = *It) E->Destroy(); }
		for (const FPalletSaveData& Row : LoadGameInstance->SavedPallets)
		{
			FActorSpawnParameters Params;
			UClass* ResolvedPalletClass = PalletClass ? *PalletClass : APalletActor::StaticClass();

			APalletActor* NewPallet = GetWorld()->SpawnActor<APalletActor>(ResolvedPalletClass, Row.Location, Row.Rotation, Params);
			if (NewPallet)
			{
				if (Row.TotalSlots > 0 && Row.TotalSlots != NewPallet->TotalSlots)
				{
					NewPallet->TotalSlots = Row.TotalSlots;
					// If needed, rebuild attachment slots here via your helper.
				}
				if (Row.SlotOccupied.Num() > 0)
				{
					NewPallet->SetSlotOccupiedArray(Row.SlotOccupied);
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to spawn pallet at %s"), *Row.Location.ToString());
			}
		}

		// Debug summary
		UE_LOG(LogTemp, Warning, TEXT("[LOAD] SavedGarageDoors in file: %d"),
			LoadGameInstance->SavedGarageDoors.Num());

		for (TActorIterator<AGarageDoorActor> It(GetWorld()); It; ++It)
		{
			AGarageDoorActor* Door = *It;
			if (!Door) continue;

			const FName ThisName = Door->GetFName();
			UE_LOG(LogTemp, Warning, TEXT("[LOAD] WorldDoor name=%s  PRE  Unlocked=%d  bDoorOpened=?"),
				*ThisName.ToString(), Door->bDoorUnlocked ? 1 : 0);

			const FGarageDoorSaveData* Found =
				LoadGameInstance->SavedGarageDoors.FindByPredicate(
					[ThisName](const FGarageDoorSaveData& D) { return D.DoorName == ThisName; });

			if (!Found)
			{
				UE_LOG(LogTemp, Warning, TEXT("[LOAD]   NO MATCH in save for %s"), *ThisName.ToString());
				continue;
			}

			UE_LOG(LogTemp, Warning, TEXT("[LOAD] WorldDoor name=%s  POST Unlocked=%d  IsOpen(FromSave)=%d  RemArrival=%.2fs"),
				*ThisName.ToString(),
				Door->bDoorUnlocked ? 1 : 0,
				Found->bIsOpen ? 1 : 0,
				Found->RemainingArrivalTime);
		}
	}
}
