// Shuyang Xing All Rights Deserve


#include "Elimination/EliminationComponent.h"

#include "Game/ShooterGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShooterPlayerState.h"
#include "ShooterTypes/ShooterTypes.h"

UEliminationComponent::UEliminationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SequentialElimInterval = 5.0f;
	LastElimTime = 0.f;
	SequentialElimCount = 0;
	StreakCount = 0;
	ElimNeededForStreak = 3;
}

void UEliminationComponent::OnRoundReported(AActor* Victim, AActor* Attacker, bool bHit, bool bHeadshot, bool bLethal)
{
	AShooterPlayerState* AttackerPlayerState = GetPlayerStateFromActor(Attacker);
	if (!IsValid(AttackerPlayerState)) return;
	ProcessHitOrMiss(bHit, AttackerPlayerState);
	if (!bHit) return;
	
	AShooterPlayerState* VictimPlayerState = GetPlayerStateFromActor(Victim);
	if (!IsValid(VictimPlayerState)) return;
	
	if (bLethal)
	{
		ProcessElimination(bHeadshot, VictimPlayerState, AttackerPlayerState);
	}
}

void UEliminationComponent::ProcessHitOrMiss(bool bHit, AShooterPlayerState* AttackerPlayerState)
{
	if (bHit)
	{
		AttackerPlayerState->AddHits();
	}
	else
	{
		AttackerPlayerState->AddMisses();
	}
}

void UEliminationComponent::ProcessElimination(bool bHeadshot, AShooterPlayerState* VictimPlayerState, AShooterPlayerState* AttackerPlayerState)
{
	AttackerPlayerState->AddScoredElims();
	VictimPlayerState->AddDefeats();
	ESpecialElimType SpecialElimType = ESpecialElimType::None;
	AShooterGameStateBase* GameState = Cast<AShooterGameStateBase>(UGameplayStatics::GetGameState(AttackerPlayerState));
	
	ProcessHeadshot(bHeadshot, SpecialElimType, AttackerPlayerState);
	ProcessSequentialElim(SpecialElimType, AttackerPlayerState);
	ProcessStreak(SpecialElimType, AttackerPlayerState, VictimPlayerState);
	if (IsValid(GameState))
	{
		HandleFirstBlood(GameState, SpecialElimType, AttackerPlayerState);
		UpdateLeaderStatus(GameState, SpecialElimType, AttackerPlayerState, VictimPlayerState);
	}
	if (HasSpecialElim(SpecialElimType))
	{
		AttackerPlayerState->Client_SpecialElim(SpecialElimType, SequentialElimCount, StreakCount, AttackerPlayerState->GetScoredElims());
	}
	else
	{
		AttackerPlayerState->Client_ScoredElim(AttackerPlayerState->GetScoredElims());
	}
}

void UEliminationComponent::ProcessHeadshot(bool bHeadshot, ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS)
{
	if (bHeadshot)
	{
		OutElimType |= ESpecialElimType::HeadShot;
		AttackerPS->AddHeadshotElims();
	}
	else
	{
		OutElimType = ESpecialElimType::None;
	}
}

void UEliminationComponent::ProcessSequentialElim(ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastElimTime <= SequentialElimInterval)
	{
		++SequentialElimCount;
	}
	else
	{
		SequentialElimCount = 1;
	}
	LastElimTime = CurrentTime;
	if (SequentialElimCount > 1)
	{
		OutElimType |= ESpecialElimType::Sequential;
		AttackerPS->AddSequentialElims(SequentialElimCount);
	}
}

void UEliminationComponent::ProcessStreak(ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS, AShooterPlayerState* VictimPS)
{
	++StreakCount;
	if (StreakCount >= ElimNeededForStreak)
	{
		OutElimType |= ESpecialElimType::Streak;
		AttackerPS->SetOnStreak(true);
		AttackerPS->UpdateHighestStreak(StreakCount);
	}
	if (VictimPS->IsOnStreak())
	{
		OutElimType |= ESpecialElimType::ShowStopper;
		AttackerPS->AddShowStopperElims();
		VictimPS->SetOnStreak(false);
	}
	if (AttackerPS->GetLastAttacker() == VictimPS)
	{
		OutElimType |= ESpecialElimType::Revenge;
		AttackerPS->AddRevengeElims();
		AttackerPS->SetLastAttacker(nullptr);
	}
	VictimPS->SetLastAttacker(AttackerPS);
}

void UEliminationComponent::HandleFirstBlood(AShooterGameStateBase* GameState, ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS)
{
	if (!GameState->HasFirstBloodOccurred())
	{
		OutElimType |= ESpecialElimType::FirstBlood;
		GameState->UpdateLeader();
		AttackerPS->GotFirstBlood();
	}
}
void UEliminationComponent::UpdateLeaderStatus(AShooterGameStateBase* GameState, ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS, AShooterPlayerState* VictimPS)
{
	AShooterPlayerState* LastLeader = GameState->GetSoleLeader();
	const bool bAttackerWasTiedForTheLead = GameState->IsTiedForTheLead(AttackerPS);
	GameState->UpdateLeader();
	if (!bAttackerWasTiedForTheLead && GameState->IsTiedForTheLead(AttackerPS))
	{
		OutElimType |= ESpecialElimType::TiedTheLeader;
	}
	if (IsValid(LastLeader) && LastLeader != GameState->GetSoleLeader())
	{
		LastLeader->Client_LostTheLead();
		
		if (VictimPS == LastLeader)
		{
			OutElimType |= ESpecialElimType::Dethrone;
			AttackerPS->AddDethroneElims();
		}
	}
	
	if (AttackerPS != LastLeader && AttackerPS == GameState->GetSoleLeader())
	{
		OutElimType |= ESpecialElimType::GainTheLead;
	}
}

bool UEliminationComponent::HasSpecialElim(const ESpecialElimType& ElimType)
{
	return static_cast<uint16>(ElimType) != 0;
}


AShooterPlayerState* UEliminationComponent::GetPlayerStateFromActor(AActor* Actor)
{
	APawn* Pawn = Cast<APawn>(Actor);
	if (IsValid(Pawn))
	{
		return Pawn->GetPlayerState<AShooterPlayerState>();
	}
	return nullptr;
}
