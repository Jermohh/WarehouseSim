#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items.h"
#include "SellZone.generated.h"


/**
* Sell zone actor
* Cleaned formatting only. No functional changes.
*/
UCLASS()
class WAREHOUSESIM_API ASellZone : public AActor
{
	GENERATED_BODY()


public:
	ASellZone();


	/** Category this zone accepts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sell Zone")
	int32 SellCategory = 1;


	UFUNCTION(BlueprintCallable, Category = "Sell Zone")
	void SellItems();


protected:
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* TriggerZone;


	UPROPERTY()
	TArray<AItems*> ItemsInZone;


	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};