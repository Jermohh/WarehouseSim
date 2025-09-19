#include "DoorSirenBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"

ADoorSirenBase::ADoorSirenBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create mesh
    SirenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SirenMesh"));
    RootComponent = SirenMesh;

    // Create light
    SirenLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SirenLight"));
    SirenLight->SetupAttachment(SirenMesh);
    SirenLight->SetLightColor(FLinearColor::Red);
    SirenLight->SetIntensity(5000.f);
}
