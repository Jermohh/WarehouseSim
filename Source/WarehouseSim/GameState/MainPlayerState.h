#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MainPlayerState.generated.h"

UCLASS()
class WAREHOUSESIM_API AMainPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMainPlayerState();

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Stats")
	int32 PlayerMoney = 0;

	UFUNCTION(BlueprintCallable, Category = "Stats") void AddMoney(int32 Amount);
	UFUNCTION(BlueprintCallable, Category = "Stats") void SpendMoney(int32 Amount);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
