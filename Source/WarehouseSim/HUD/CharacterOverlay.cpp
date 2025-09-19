// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"
#include "Components/TextBlock.h"


void UCharacterOverlay::SetMoneyText(int32 NewMoneyAmount)
{
	if (Money)
	{
		Money->SetText(FText::AsNumber(NewMoneyAmount));
	}
}