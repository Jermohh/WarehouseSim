#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lights.generated.h"

UCLASS()
class WAREHOUSESIM_API ALights : public AActor
{
	GENERATED_BODY()

public:
	ALights();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	class UPointLightComponent* PointLight;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* InteractionBox;

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
		class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void ToggleLight();

	bool bCanToggle = false;
};
