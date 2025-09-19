#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WarehouseAnimInstance.generated.h"

UCLASS()
class WAREHOUSESIM_API UWarehouseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	/** IK Targets */
	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector RightHandIKTarget;

	UPROPERTY(BlueprintReadOnly, Category = "IK")
	FVector LeftHandIKTarget;

protected:
	/** Owning Character */
	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	class AWarehouseCharacter* WarehouseCharacter;

	/** Movement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FRotator MovementOffsetRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FRotator LastMovementOffsetRotation;

	/** Cargo States */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCarryingCargo;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCarryingPallet;

private:
	FVector Velocity;
	FVector Acceleration;
};
