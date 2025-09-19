// Fill out your copyright notice in the Description page of Project Settings.
#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"


/**
* Character HUD overlay widget
* Cleaned formatting only. No functional changes.
*/
UCLASS()
class WAREHOUSESIM_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable)
	void SetMoneyText(int32 NewMoneyAmount);


protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Money;
};