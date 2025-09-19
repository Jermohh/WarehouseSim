#include "ComputerTerminal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "WarehouseSim/Character/WarehouseCharacter.h"

/* ------------------ Construction ------------------ */

AComputerTerminal::AComputerTerminal()
{
	PrimaryActorTick.bCanEverTick = true;
}

/* ------------------ BeginPlay ------------------ */

void AComputerTerminal::BeginPlay()
{
	Super::BeginPlay();
}

/* ------------------ Tick ------------------ */

void AComputerTerminal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}