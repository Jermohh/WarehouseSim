#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"

#include "DoorSirenBase.generated.h"

UCLASS()
class WAREHOUSESIM_API ADoorSirenBase : public AActor
{
    GENERATED_BODY()

public:
    ADoorSirenBase();

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* SirenMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UPointLightComponent* SirenLight;

    UFUNCTION(BlueprintImplementableEvent, Category = "Siren")
    void TriggerColorChange();

    UFUNCTION(BlueprintImplementableEvent, Category = "Siren")
    void ResetTriggerColorChange();
};
