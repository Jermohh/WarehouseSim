#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ComputerTerminal.generated.h"

class UStaticMeshComponent;
class AWarehouseCharacter;

UCLASS()
class WAREHOUSESIM_API AComputerTerminal : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	AComputerTerminal();

	// Engine overrides
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Components */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

};