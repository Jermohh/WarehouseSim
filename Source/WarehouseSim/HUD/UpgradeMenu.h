#pragma once


#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeMenu.generated.h"


/**
* Upgrade Menu widget
* Cleaned formatting only. No functional changes.
*/
UCLASS()
class WAREHOUSESIM_API UUpgradeMenu : public UUserWidget
{
	GENERATED_BODY()


public:
	UFUNCTION(BlueprintCallable)
	void CloseMenu();
};