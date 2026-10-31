// Shuyang Xing All Rights Deserve


#include "Game/ShooterGameStateBase.h"

#include "Player/ShooterPlayerState.h"

AShooterGameStateBase::AShooterGameStateBase()
{
	bHasFirstBloodOccurred = false;
}

void AShooterGameStateBase::UpdateLeader()
{
	TArray<APlayerState*> LocalSortedPlayers = PlayerArray;
	LocalSortedPlayers.Sort([](const APlayerState& A, const APlayerState& B)
	{
		const AShooterPlayerState* ShooterA = Cast<AShooterPlayerState>(&A);
		const AShooterPlayerState* ShooterB = Cast<AShooterPlayerState>(&B);
		return ShooterA && ShooterB && ShooterA->GetScoredElims() > ShooterB->GetScoredElims();
	});
	
	Leaders.Empty();
	if (LocalSortedPlayers.Num() > 0)
	{
		int32 HighestScore = 0;
		for (APlayerState* PlayerState : LocalSortedPlayers)
		{
			AShooterPlayerState* ShooterPlayerState = Cast<AShooterPlayerState>(PlayerState);
			int32 PlayerScore = ShooterPlayerState->GetScoredElims();
			if (Leaders.Num() == 0)
			{
				HighestScore = PlayerScore;
				Leaders.Add(ShooterPlayerState);
			}
			else if (PlayerScore == HighestScore)
			{
				Leaders.Add(ShooterPlayerState);
			}
			else
			{
				break;
			}
		}
	}
	
	bHasFirstBloodOccurred = true;
}

AShooterPlayerState* AShooterGameStateBase::GetSoleLeader()
{
	if (Leaders.Num() == 1)
	{
		return Leaders[0];
	}
	return nullptr;
}

bool AShooterGameStateBase::IsTiedForTheLead(AShooterPlayerState* PlayerState)
{
	return Leaders.Contains(PlayerState) && Leaders.Num() > 1;
}
