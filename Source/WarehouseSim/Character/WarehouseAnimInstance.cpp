#include "WarehouseAnimInstance.h"
#include "WarehouseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "WarehouseSim/Components/CargoComponent.h"
#include "WarehouseSim/Items/Items.h"
#include "WarehouseSim/Items/PalletActor.h"

void UWarehouseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	WarehouseCharacter = Cast<AWarehouseCharacter>(TryGetPawnOwner());
}

void UWarehouseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!WarehouseCharacter)
	{
		WarehouseCharacter = Cast<AWarehouseCharacter>(TryGetPawnOwner());
	}

	if (!WarehouseCharacter) return;

	// Movement
	Velocity = WarehouseCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsFalling = WarehouseCharacter->GetCharacterMovement()->IsFalling();
	Acceleration = WarehouseCharacter->GetCharacterMovement()->GetCurrentAcceleration();
	bIsAccelerating = Acceleration.Size() > 0.f;

	FRotator AimRotation = WarehouseCharacter->GetActorRotation();
	FRotator MovementRotation = Velocity.ToOrientationRotator();
	MovementOffsetRotation = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);

	FVector Forward = FRotationMatrix(AimRotation).GetUnitAxis(EAxis::X);
	FVector Right = FRotationMatrix(AimRotation).GetUnitAxis(EAxis::Y);
	FVector NormalizedVel = Velocity.GetSafeNormal();

	float ForwardDot = FVector::DotProduct(Forward, NormalizedVel);
	float RightDot = FVector::DotProduct(Right, NormalizedVel);

	Direction = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

	if (Speed > 0.f)
	{
		LastMovementOffsetRotation = MovementOffsetRotation;
	}

	// Cargo states
	UCargoComponent* CargoComp = WarehouseCharacter->FindComponentByClass<UCargoComponent>();
	if (!CargoComp) return;

	bIsCarryingCargo = (CargoComp->GetHeldItem() != nullptr);
	bIsCarryingPallet = (CargoComp->GetHeldPallet() != nullptr);

	if (AItems* HeldItem = CargoComp->GetHeldItem())
	{
		RightHandIKTarget = HeldItem->GetHandIKSocketLocation();
		LeftHandIKTarget = HeldItem->GetLeftHandIKSocketLocation();
	}
	else if (APalletActor* HeldPallet = CargoComp->GetHeldPallet())
	{
		RightHandIKTarget = HeldPallet->GetHandIKSocketLocation();
		LeftHandIKTarget = HeldPallet->GetLeftHandIKSocketLocation();
	}
	else
	{
		RightHandIKTarget = FVector::ZeroVector;
		LeftHandIKTarget = FVector::ZeroVector;
	}
}
