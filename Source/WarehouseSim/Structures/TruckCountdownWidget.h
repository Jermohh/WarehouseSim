#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TruckCountdownWidget.generated.h"

class AGarageDoorActor;

UCLASS()
class WAREHOUSESIM_API UTruckCountdownWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Countdown")
	AGarageDoorActor* LinkedDoor;

	UFUNCTION(BlueprintImplementableEvent, Category = "Countdown")
	void UpdateCountdown(float TimeLeft);
};
