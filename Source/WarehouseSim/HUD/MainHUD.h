// Fill out your copyright notice in the Description page of Project Settings.
#pragma once


#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainHUD.generated.h"


/**
* Player HUD manager
* Cleaned formatting only. No functional changes.
*/
UCLASS()
class WAREHOUSESIM_API AMainHUD : public AHUD
{
	GENERATED_BODY()


public:
	virtual void BeginPlay() override;


	/** Overlay widget */
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<class UCharacterOverlay> CharacterOverlayClass;


	/** Instance of the overlay */
	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
};