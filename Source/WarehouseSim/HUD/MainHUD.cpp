// Fill out your copyright notice in the Description page of Project Settings.


#include "MainHUD.h"
#include "Blueprint/UserWidget.h"
#include "WarehouseSim/HUD/CharacterOverlay.h"
#include "GameFramework/PlayerController.h"


void AMainHUD::BeginPlay()
{
	Super::BeginPlay();


	if (CharacterOverlayClass)
	{
		APlayerController* PC = GetOwningPlayerController();
		if (PC)
		{
			CharacterOverlay = CreateWidget<UCharacterOverlay>(PC, CharacterOverlayClass);
			if (CharacterOverlay)
			{
				CharacterOverlay->AddToViewport();
				UE_LOG(LogTemp, Warning, TEXT("CharacterOverlay added to viewport from MainHUD"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("CharacterOverlay creation failed in MainHUD"));
			}
		}
	}
}