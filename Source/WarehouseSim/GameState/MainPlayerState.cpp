#include "MainPlayerState.h"
#include "Net/UnrealNetwork.h"

AMainPlayerState::AMainPlayerState()
{
	PlayerMoney = 0;
}

void AMainPlayerState::AddMoney(int32 Amount)
{
	PlayerMoney += Amount;
}

void AMainPlayerState::SpendMoney(int32 Amount)
{
	PlayerMoney = FMath::Max(0, PlayerMoney - Amount);
}

void AMainPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMainPlayerState, PlayerMoney);
}
