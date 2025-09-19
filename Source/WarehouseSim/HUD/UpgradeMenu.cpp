// Fill out your copyright notice in the Description page of Project Settings.


#include "UpgradeMenu.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"


void UUpgradeMenu::CloseMenu()
{
	RemoveFromParent();


	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}