#include "WarehouseController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/HUD.h"
#include "WarehouseSim/HUD/CharacterOverlay.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"
#include "WarehouseSim/HUD/MainHUD.h"


void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay();

    AMainHUD* HUD = Cast<AMainHUD>(GetHUD());
    if (HUD && HUD->CharacterOverlay)
    {
        // Optionally set default values here if needed
        UE_LOG(LogTemp, Warning, TEXT("BeginPlay: HUD and CharacterOverlay are valid."));
    }

}

void AMainPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
}

void AMainPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AMainPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    UE_LOG(LogTemp, Warning, TEXT("OnPossess called in AMainPlayerController"));

    AWarehouseCharacter* WarehouseChar = Cast<AWarehouseCharacter>(InPawn);
    if (WarehouseChar)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPossess: Possessed %s"), *WarehouseChar->GetName());
        // Ask the character to update HUD
        WarehouseChar->UpdateHUDMoney();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OnPossess: Failed to cast to AWarehouseCharacter"));
    }

    AMainHUD* HUD = Cast<AMainHUD>(GetHUD());
    if (HUD)
    {
        UE_LOG(LogTemp, Warning, TEXT("OnPossess: HUD is valid"));
        if (!HUD->CharacterOverlay)
        {
            UE_LOG(LogTemp, Warning, TEXT("OnPossess: CharacterOverlay is still nullptr"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OnPossess: Failed to cast GetHUD() to AMainHUD"));
    }
}

void AMainPlayerController::SetHUDMoney(int32 NewMoney)
{
    AMainHUD* HUD = Cast<AMainHUD>(GetHUD());
    if (HUD && HUD->CharacterOverlay)
    {
        HUD->CharacterOverlay->SetMoneyText(NewMoney);
        UE_LOG(LogTemp, Warning, TEXT("SetHUDMoney: HUD updated with value: %d"), NewMoney);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SetHUDMoney: HUD or CharacterOverlay not valid"));
    }
}


