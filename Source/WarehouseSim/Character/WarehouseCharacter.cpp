#include "WarehouseCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WarehouseSim/Components/CargoComponent.h"
#include "WarehouseSim/Items/Items.h"
#include "Components/CapsuleComponent.h"
#include "WarehouseSim/Items/PalletActor.h"
#include "WarehouseSim/Controller/WarehouseController.h"
#include "WarehouseSim/Structures/DoorButtonActor.h"
#include "WarehouseSim/Structures/GarageDoorActor.h"
#include "WarehouseSim/HUD/UpgradeMenu.h"
#include "Kismet/GameplayStatics.h"
#include "WarehouseSim/Structures/ConveyorBelt.h"
#include "Blueprint/UserWidget.h"
#include "WarehouseSim/GameState/MainGameState.h"
#include "WarehouseSim/Items/SellZone.h"
#include "WarehouseSim/Structures/SellZoneButtonActor.h"
#include "WarehouseSim/Structures/GarageSellDoorActor.h"
#include "WarehouseSim/Structures/SellingTruckActor.h"
#include "Components/BoxComponent.h"
#include "WarehouseSim/GameState/MainPlayerState.h"
#include "WarehouseSim/Items/ScannerStation.h"


AWarehouseCharacter::AWarehouseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	CargoComponent = CreateDefaultSubobject<UCargoComponent>(TEXT("CargoComponent"));

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ThirdPersonMesh"));
	ThirdPersonMesh->SetupAttachment(GetCapsuleComponent());
	ThirdPersonMesh->SetOnlyOwnerSee(false);
	ThirdPersonMesh->SetOwnerNoSee(true);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->SetOwnerNoSee(false);
	
	HandAttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HandAttachPoint"));
	HandAttachPoint->SetupAttachment(GetThirdPersonMesh(), FName("CarrySocket"));
	HandAttachPoint->SetWorldScale3D(FVector(10.f));
	HandAttachPoint->SetHiddenInGame(false);
}

void AWarehouseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		FirstPersonMesh->SetVisibility(true, true);
		GetMesh()->SetVisibility(false, true);
	}
	else
	{
		FirstPersonMesh->SetVisibility(false, true);
		GetMesh()->SetVisibility(true, true);
	}

	if (HasAuthority())
	{
		if (AMainPlayerState* PS = GetPlayerState<AMainPlayerState>())
		{
			Money = PS->PlayerMoney;
		}
		if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
		{
			if (!GS->bHasLoadedSave)
			{
				GS->LoadGameData();
				GS->bHasLoadedSave = true;
			}
		}
	}
}

void AWarehouseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWarehouseCharacter, Money);
	DOREPLIFETIME(AWarehouseCharacter, HeldBelt);
	DOREPLIFETIME(AWarehouseCharacter, TruckUpgradeLevel);
	DOREPLIFETIME(AWarehouseCharacter, bInRangeOfComputer);
	DOREPLIFETIME(AWarehouseCharacter, HeldScanner);
}

void AWarehouseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWarehouseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AWarehouseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWarehouseCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AWarehouseCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AWarehouseCharacter::LookUp);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &AWarehouseCharacter::OnPickupPressed);
	PlayerInputComponent->BindAction("PlaceItem", IE_Pressed, this, &AWarehouseCharacter::OnPlaceItemPressed);
	PlayerInputComponent->BindAction("HeavyObjectPickup", IE_Pressed, this, &AWarehouseCharacter::OnHeavyObjectPickupPressed);
	PlayerInputComponent->BindAction("CloseDoor", IE_Pressed, this, &AWarehouseCharacter::HandleDoorClose);
	PlayerInputComponent->BindAction("BuyDoor", IE_Pressed, this, &AWarehouseCharacter::OnBuyDoorPressed);
	PlayerInputComponent->BindAction("ToggleUpgradeMenu", IE_Pressed, this, &AWarehouseCharacter::ToggleUpgradeMenu);
	PlayerInputComponent->BindAction("EditConveyor", IE_Pressed, this, &AWarehouseCharacter::OnEditConveyorPressed);
	PlayerInputComponent->BindAction("Sell", IE_Pressed, this, &AWarehouseCharacter::OnSellZoneButtonPressed);
	PlayerInputComponent->BindAction("SaveGame", IE_Pressed, this, &AWarehouseCharacter::HandleSaveGame);
	PlayerInputComponent->BindAction("LoadGame", IE_Pressed, this, &AWarehouseCharacter::HandleLoadGame);
	PlayerInputComponent->BindAction("ClearSave", IE_Pressed, this, &AWarehouseCharacter::HandleClearSave);
	PlayerInputComponent->BindAction("PickupScanner", IE_Pressed, this, &AWarehouseCharacter::OnEditScannerPressed);
}

void AWarehouseCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		if (Value < 0.f)
		{
			GetCharacterMovement()->MaxWalkSpeed = 300.f;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = 600.f;
		}

		float NewSpeed = Value < 0.f ? 300.f : 600.f;

		GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

		if (!HasAuthority())
		{
			ServerSetWalkSpeed(NewSpeed);
		}

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AWarehouseCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AWarehouseCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AWarehouseCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AWarehouseCharacter::OnPickupPressed()
{
	if (!CargoComponent) return;

	FHitResult Hit;
	TraceUnderCrosshair(Hit);

	if (Hit.bBlockingHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("LineTrace hit actor: %s"), *GetNameSafe(Hit.GetActor()));
		DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(10.f), FColor::Red, false, 2.f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LineTrace did not hit anything"));
	}


	if (CargoComponent->IsHoldingItem())
	{
		APalletActor* HitPallet = Cast<APalletActor>(Hit.GetActor());
		if (HitPallet)
		{
			UE_LOG(LogTemp, Warning, TEXT("Attempting to place item on pallet: %s"), *HitPallet->GetName());
			CargoComponent->TryPlaceItemOnPallet();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Not looking at pallet — dropping item instead."));
			CargoComponent->DropItem();
		}
	}
	else
	{
		AItems* HitItem = Cast<AItems>(Hit.GetActor());
		if (HitItem && !HitItem->IsHeld())
		{
			UE_LOG(LogTemp, Warning, TEXT("Picking up item: %s"), *GetNameSafe(HitItem));
			CargoComponent->PickupItem(HitItem);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No valid item found or item is already held"));
		}
	}


}

void AWarehouseCharacter::OnHeavyObjectPickupPressed()
{
	if (!CargoComponent) return;

	if (CargoComponent->IsHoldingPallet())
	{
		UE_LOG(LogTemp, Warning, TEXT("Heavy Pickup: Dropping pallet"));
		CargoComponent->DropPallet();
	}
	else
	{
		FHitResult Hit;
		TraceUnderCrosshair(Hit);

		APalletActor* Pallet = Cast<APalletActor>(Hit.GetActor());
		if (Pallet)
		{
			UE_LOG(LogTemp, Warning, TEXT("Heavy Pickup: Picking up pallet %s"), *Pallet->GetName());
			CargoComponent->PickupPallet(Pallet);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Heavy Pickup: No pallet under crosshair"));
		}
	}
}

void AWarehouseCharacter::OnPlaceItemPressed()
{
	if (CargoComponent)
	{
		CargoComponent->TryPlaceItemOnPallet();
	}
}

void AWarehouseCharacter::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	FVector Start = FirstPersonCamera->GetComponentLocation();
	FVector End = Start + (FirstPersonCamera->GetForwardVector() * 500.f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECC_Visibility, Params);

	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.f, 0, 1.f);
}


void AWarehouseCharacter::ServerSetWalkSpeed_Implementation(float NewSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void AWarehouseCharacter::UpdateHUDMoney()
{
	if (AMainPlayerController* PC = Cast<AMainPlayerController>(Controller))
	{
		PC->SetHUDMoney(Money);
	}

	if (AMainPlayerState* PS = GetPlayerState<AMainPlayerState>())
	{
		PS->PlayerMoney = Money;
	}
}

void AWarehouseCharacter::OnRep_Money()
{
	UpdateHUDMoney();
}

void AWarehouseCharacter::HandleDoorClose()
{
	if (OverlappingDoorButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("ButtonInputisbeingCalled"));
		Server_RequestDoorClose(OverlappingDoorButton);
	}
}

void AWarehouseCharacter::Server_RequestDoorClose_Implementation(ADoorButtonActor* Button)
{
	if (Button)
	{
		Button->RequestDoorClose();
	}
}

void AWarehouseCharacter::OnBuyDoorPressed()
{
	FHitResult Hit;
	TraceUnderCrosshair(Hit);

	AGarageDoorActor* Door = Cast<AGarageDoorActor>(Hit.GetActor());
	if (Door)
	{
		ServerBuyGarageDoor(Door);
	}
}

void AWarehouseCharacter::ServerBuyGarageDoor_Implementation(AGarageDoorActor* Door)
{
	if (Door)
	{
		Door->ServerBuyDoor(this);
	}
}

void AWarehouseCharacter::ToggleUpgradeMenu()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;
	if (!bInRangeOfComputer)
	{
		UE_LOG(LogTemp, Warning, TEXT("Must be near computer to open upgrades."));
		return;
	}

	if (bIsUpgradeMenuOpen)
	{
		if (UpgradeMenuInstance)
		{
			UpgradeMenuInstance->RemoveFromParent();
			UpgradeMenuInstance = nullptr;
		}

		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
		bIsUpgradeMenuOpen = false;
	}
	else
	{
		if (!UpgradeMenuClass) return;

		UpgradeMenuInstance = CreateWidget<UUpgradeMenu>(PC, UpgradeMenuClass);
		if (UpgradeMenuInstance)
		{
			UpgradeMenuInstance->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(UpgradeMenuInstance->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
			bIsUpgradeMenuOpen = true;
		}
	}
}

void AWarehouseCharacter::ApplyConveyorUpgrade()
{
	if (HasAuthority())
	{
		Server_RequestConveyorUpgrade();
	}
	else
	{
		Server_RequestConveyorUpgrade();
	}
}



void AWarehouseCharacter::Server_RequestConveyorUpgrade_Implementation()
{
	AMainGameState* GS = Cast<AMainGameState>(UGameplayStatics::GetGameState(this));
	if (!GS) return;

	int32 Cost = GS->BaseConveyorUpgradeCost + (GS->ConveyorUpgradeLevel * 25);

	if (Money >= Cost)
	{
		Money -= Cost;
		GS->UpgradeConveyorSpeed();
		UpdateHUDMoney();
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Upgrade applied. Money left: %d"), Money);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Not enough money to upgrade."));
	}
}

void AWarehouseCharacter::ApplyTruckUpgrade()
{
	if (HasAuthority())
	{
		Server_RequestTruckUpgrade();
	}
	else
	{
		Server_RequestTruckUpgrade();
	}
}

void AWarehouseCharacter::Server_RequestTruckUpgrade_Implementation()
{
	AMainGameState* GS = Cast<AMainGameState>(UGameplayStatics::GetGameState(this));
	if (!GS) return;

	const int32 Cost = 100 + (GS->TruckUpgradeLevel * 50);

	if (Money >= Cost)
	{
		Money -= Cost;
		GS->UpgradeTruckBoxSpawn();
		UpdateHUDMoney();
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Global Truck Upgrade applied. New Level: %d"), GS->TruckUpgradeLevel);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Not enough money to upgrade truck."));
	}
}

void AWarehouseCharacter::ApplyTruckSpawnTimeUpgrade()
{
	
	Server_RequestTruckSpawnTimeUpgrade();
}

void AWarehouseCharacter::Server_RequestTruckSpawnTimeUpgrade_Implementation()
{
	AMainGameState* GS = Cast<AMainGameState>(UGameplayStatics::GetGameState(this));
	if (!GS) return;

	const int32 Cost = GS->TruckSpawnTimeUpgradeCost;

	if (Money >= Cost)
	{
		Money -= Cost;
		UpdateHUDMoney();

		GS->UpgradeTruckSpawnTime(); 

		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Truck SpawnTime Upgrade purchased. New Level: %d Mult: %.2f NextCost: %d"),
			GS->TruckSpawnTimeUpgradeLevel, GS->TruckSpawnTimeMultiplier, GS->TruckSpawnTimeUpgradeCost);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Not enough money for Truck SpawnTime upgrade. Need %d, have %d"), Cost, Money);
	}
}

void AWarehouseCharacter::ApplyScannerValueUpgrade()
{
	Server_RequestScannerValueUpgrade();
}

void AWarehouseCharacter::Server_RequestScannerValueUpgrade_Implementation()
{
	AMainGameState* GS = Cast<AMainGameState>(UGameplayStatics::GetGameState(this));
	if (!GS) return;

	const int32 Cost = GS->ScannerValueUpgradeCost;

	if (Money >= Cost)
	{
		Money -= Cost;
		UpdateHUDMoney();
		GS->UpgradeScannerValue(); 
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Scanner upgrade bought. L:%d  Min:+%.0f%%  Max:+%.0f%%  NextCost:%d"),
			GS->ScannerValueUpgradeLevel, GS->ScannerBonusMinPct * 100.f, GS->ScannerBonusMaxPct * 100.f, GS->ScannerValueUpgradeCost);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Not enough money for scanner upgrade. Need %d, have %d"), Cost, Money);
	}
}

void AWarehouseCharacter::OnEditConveyorPressed()
{
	FHitResult Hit;
	TraceUnderCrosshair(Hit);

	if (!HeldBelt)
	{
		if (AConveyorBelt* TargetBelt = Cast<AConveyorBelt>(Hit.GetActor()))
		{
			if (HasAuthority())
			{
				TargetBelt->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				HeldBelt = TargetBelt;
				MulticastAttachConveyor(TargetBelt);
			}
			else
			{
				ServerPickupConveyor(TargetBelt);
			}
		}
		return;
	}

	if (AConveyorBelt* TargetBelt = Cast<AConveyorBelt>(Hit.GetActor()))
	{
		const FVector HitPoint = Hit.ImpactPoint;
		UBoxComponent* ClosestSnapBox = TargetBelt->GetClosestSnapBox(HitPoint);
		if (ClosestSnapBox)
		{
			const FTransform SnapTransform = ClosestSnapBox->GetComponentTransform();

			FRotator PlaceRotation = SnapTransform.GetRotation().Rotator();

			bool bTurned = false;
			if (ClosestSnapBox == TargetBelt->SnapFrontLeft)
			{
				PlaceRotation.Yaw += -90.f;
				bTurned = true;
			}
			else if (ClosestSnapBox == TargetBelt->SnapFrontRight)
			{
				PlaceRotation.Yaw += 90.f;
				bTurned = true;
			}

			if (!bTurned)
			{
				if (ClosestSnapBox->ComponentHasTag(FName("SnapFrontLeft")))
				{
					PlaceRotation.Yaw += -90.f;
				}
				else if (ClosestSnapBox->ComponentHasTag(FName("SnapFrontRight")))
				{
					PlaceRotation.Yaw += 90.f;
				}
			}

			ServerSnapPlaceConveyor(HeldBelt, SnapTransform.GetLocation(), PlaceRotation);
			return;
		}
	}

	const FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0.f, 0.f, 50.f);
	ServerDropConveyor(DropLocation);
}

void AWarehouseCharacter::OnRep_HeldBelt()
{
	if (HeldBelt)
	{
		MulticastAttachConveyor(HeldBelt);
	}
}

void AWarehouseCharacter::ServerPickupConveyor_Implementation(AConveyorBelt* Belt)
{
	if (!Belt || HeldBelt) return;

	Belt->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	HeldBelt = Belt;
	MulticastAttachConveyor(Belt);
}

void AWarehouseCharacter::ServerDropConveyor_Implementation(const FVector& DropLocation)
{
	if (!HeldBelt) return;

	HeldBelt->SetActorLocation(DropLocation);
	MulticastDetachConveyor(HeldBelt);
	HeldBelt = nullptr;
}

void AWarehouseCharacter::ServerSnapPlaceConveyor_Implementation(AConveyorBelt* Belt, const FVector& Location, const FRotator& Rotation)
{
	if (!Belt) return;

	Belt->SetActorLocation(Location);
	Belt->SetActorRotation(Rotation);

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(Belt->GetRootComponent()))
	{
		RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RootPrim->SetCollisionResponseToAllChannels(ECR_Block);
		RootPrim->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		RootPrim->SetSimulatePhysics(true);
	}

	Belt->SetActorEnableCollision(true);
	MulticastDetachConveyor(Belt);
	HeldBelt = nullptr;
}

void AWarehouseCharacter::MulticastAttachConveyor_Implementation(AConveyorBelt* Belt)
{
	if (!Belt) return;

	UPrimitiveComponent* PrimitiveRoot = Cast<UPrimitiveComponent>(Belt->GetRootComponent());
	if (PrimitiveRoot)
	{
		PrimitiveRoot->SetSimulatePhysics(false);
		PrimitiveRoot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Belt->SetActorEnableCollision(false);
	Belt->AttachToComponent(FirstPersonCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Belt->SetActorRelativeLocation(FVector(50.f, 0.f, -15.f));
	Belt->SetActorRelativeRotation(FRotator::ZeroRotator);
}

void AWarehouseCharacter::MulticastDetachConveyor_Implementation(AConveyorBelt* Belt)
{
	if (!Belt) return;

	Belt->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Belt->SetActorEnableCollision(true);
	if (Belt->GetRootComponent())
	{
		UPrimitiveComponent* PrimitiveRoot = Cast<UPrimitiveComponent>(Belt->GetRootComponent());
		if (PrimitiveRoot)
		{
			PrimitiveRoot->SetSimulatePhysics(true);
			PrimitiveRoot->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			PrimitiveRoot->SetCollisionResponseToAllChannels(ECR_Block);
			PrimitiveRoot->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}
	}
}

void AWarehouseCharacter::BuyScannerStation()
{
	if (!HasAuthority())
	{
		Server_BuyScannerStation();
		return;
	}
	Server_BuyScannerStation();
}

void AWarehouseCharacter::Server_BuyScannerStation_Implementation()
{
	AMainGameState* GS = Cast<AMainGameState>(UGameplayStatics::GetGameState(this));
	if (!GS) return;

	const int32 Cost = GS->ScannerStationCost;

	if (Money >= Cost)
	{
		Money -= Cost;
		UpdateHUDMoney();

		if (ScannerStationClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			GetWorld()->SpawnActor<AScannerStation>(ScannerStationClass, ConveyorSpawnLocation, ConveyorSpawnRotation, SpawnParams);
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] Conveyor belt bought for %d. Money left: %d"), Cost, Money);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] ConveyorBeltClass not set on %s"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Not enough money for conveyor belt. Need %d, have %d"), Cost, Money);
	}
}

void AWarehouseCharacter::BuyPallet()
{
	if (!HasAuthority())
	{
		Server_BuyPallet();
		return;
	}
	Server_BuyPallet();
}

void AWarehouseCharacter::Server_BuyPallet_Implementation()
{
	AMainGameState* GS = Cast<AMainGameState>(UGameplayStatics::GetGameState(this));
	if (!GS) return;

	const int32 Cost = GS->PalletCost;

	if (Money >= Cost)
	{
		Money -= Cost;
		UpdateHUDMoney();

		if (PalletClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = this;
			SpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			const FVector Loc = (PalletSpawnLocation.IsZero() ? ConveyorSpawnLocation : PalletSpawnLocation);
			const FRotator Rot = (PalletSpawnRotation.IsZero() ? ConveyorSpawnRotation : PalletSpawnRotation);

			APalletActor* NewPallet = GetWorld()->SpawnActor<APalletActor>(PalletClass, Loc, Rot, SpawnParams);

			if (NewPallet)
			{
				UE_LOG(LogTemp, Warning, TEXT("[SERVER] Pallet bought for %d. Money left: %d"), Cost, Money);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[SERVER] Failed to spawn Pallet (class unset or blocked)."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] PalletClass not set on %s"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Not enough money for pallet. Need %d, have %d"), Cost, Money);
	}
}

void AWarehouseCharacter::BuyConveyorBelt()
{
	if (!HasAuthority())
	{
		Server_BuyConveyorBelt();
		return;
	}
	Server_BuyConveyorBelt();
}

void AWarehouseCharacter::Server_BuyConveyorBelt_Implementation()
{
	AMainGameState* GS = Cast<AMainGameState>(UGameplayStatics::GetGameState(this));
	if (!GS) return;

	const int32 Cost = GS->ConveyorBeltCost;

	if (Money >= Cost)
	{
		Money -= Cost;
		UpdateHUDMoney();

		if (ConveyorBeltClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.Instigator = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			GetWorld()->SpawnActor<AConveyorBelt>(ConveyorBeltClass, ConveyorSpawnLocation, ConveyorSpawnRotation, SpawnParams);
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] Conveyor belt bought for %d. Money left: %d"), Cost, Money);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[SERVER] ConveyorBeltClass not set on %s"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SERVER] Not enough money for conveyor belt. Need %d, have %d"), Cost, Money);
	}
}

void AWarehouseCharacter::ServerRequestSellZoneSell_Implementation(ASellZone* SellZone)
{
	if (SellZone)
	{
		SellZone->SellItems();
	}
}

void AWarehouseCharacter::OnSellZoneButtonPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("[Character] Sell button key was pressed"));

	if (OverlappingSellZoneButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Character] Overlapping SellZoneButton: %s"), *OverlappingSellZoneButton->GetName());

		if (OverlappingSellZoneButton->LinkedSellZone)
		{
			ServerRequestSellZoneSell(OverlappingSellZoneButton->LinkedSellZone);
		}

		if (OverlappingSellZoneButton->LinkedGarageDoor)
		{
			ServerActivateGarageDoor(OverlappingSellZoneButton->LinkedGarageDoor);
		}

		if (OverlappingSellZoneButton->LinkedSellingTruckActor)
		{
			ServerActivateSellingTruck(OverlappingSellZoneButton->LinkedSellingTruckActor);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Character] Not overlapping any SellZoneButton"));
	}
}

void AWarehouseCharacter::ServerActivateGarageDoor_Implementation(AGarageSellDoorActor* Door)
{
	if (Door)
	{
		Door->ActivateSell();
	}
}

void AWarehouseCharacter::ServerActivateSellingTruck_Implementation(ASellingTruckActor* Truck)
{
	if (Truck)
	{
		Truck->ActivateTruck();
	}
}

void AWarehouseCharacter::HandleClearSave()
{
	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		GS->ClearSave();
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
				TEXT("NO AMainGameState found. Check map GameMode->GameStateClass!"));
		}
	}
}

void AWarehouseCharacter::HandleSaveGame()
{
	UE_LOG(LogTemp, Warning, TEXT("handlesavegameiscalled"));
	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleSaveGame"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f,
			HasAuthority() ? FColor::Green : FColor::Yellow,
			*FString::Printf(TEXT("Calling SaveGameData. Char HasAuthority=%d  GS=%s"),
				HasAuthority() ? 1 : 0, *GetNameSafe(GS)));
		GS->SaveGameData();
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			TEXT("NO AMainGameState. Set GameMode->GameStateClass to AMainGameState!"));
	}
}

void AWarehouseCharacter::HandleLoadGame()
{
	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f,
			HasAuthority() ? FColor::Green : FColor::Red,
			*FString::Printf(TEXT("Save pressed. Char HasAuthority=%d  GS=%s"),
				HasAuthority() ? 1 : 0, *GetNameSafe(GS)));

		GS->LoadGameData();
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			TEXT("NO AMainGameState found. Check map GameMode->GameStateClass!"));
	}
}

void AWarehouseCharacter::OnRep_HeldScanner()
{
	if (HeldScanner)
	{
		HeldScanner->Multicast_OnPickedUp(FirstPersonCamera);
	}
}

void AWarehouseCharacter::ServerPickupScanner_Implementation(AScannerStation* Scanner)
{
	if (!Scanner || HeldScanner) return;
	HeldScanner = Scanner;
	Scanner->Multicast_OnPickedUp(FirstPersonCamera);
}

void AWarehouseCharacter::ServerDropScanner_Implementation(const FVector& DropLocation)
{
	if (!HeldScanner) return;
	HeldScanner->SetActorLocation(DropLocation);
	HeldScanner->Multicast_OnDropped();
	HeldScanner = nullptr;
}

void AWarehouseCharacter::ServerAttachScannerCenter_Implementation(AScannerStation* Scanner, AConveyorBelt* Belt)
{
	if (!Scanner || !Belt || !Belt->ScannerMountCenter) return;
	Scanner->Server_AttachToMount(Belt->ScannerMountCenter);

	if (Scanner == HeldScanner) { HeldScanner = nullptr; }
}

void AWarehouseCharacter::OnEditScannerPressed()
{
	FHitResult Hit;
	TraceUnderCrosshair(Hit);

	if (!HeldScanner)
	{
		if (AScannerStation* Scanner = Cast<AScannerStation>(Hit.GetActor()))
		{
			ServerPickupScanner(Scanner);
			return;
		}
	}

	if (HeldScanner)
	{
		if (AConveyorBelt* Belt = Cast<AConveyorBelt>(Hit.GetActor()))
		{
			ServerAttachScannerCenter(HeldScanner, Belt);
			return;
		}

		const FVector DropLoc = GetActorLocation() + GetActorForwardVector() * 100.f + FVector(0, 0, 40.f);
		ServerDropScanner(DropLoc);
	}
}