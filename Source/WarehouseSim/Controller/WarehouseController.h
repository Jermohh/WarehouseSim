#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WarehouseController.generated.h"

UCLASS()
class WAREHOUSESIM_API AMainPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaTime) override;
    void OnPossess(APawn* InPawn);

    void SetHUDMoney(int32 NewMoney);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    TSubclassOf<class UCharacterOverlay> CharacterOverlayClass;

    UPROPERTY(BlueprintReadOnly)
    UCharacterOverlay* CharacterOverlay;
};