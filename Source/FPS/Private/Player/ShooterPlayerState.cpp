// Shuyang Xing All Rights Deserve


#include "Player/ShooterPlayerState.h"

#include "Data/SpecialElimData.h"
#include "UI/SpecialElim.h"

AShooterPlayerState::AShooterPlayerState()
{
	SetNetUpdateFrequency(100.f);
	ScoredElims = 0;
	Defeats = 0;
	Hits = 0;
	Misses = 0;
	HeadshotElims = 0;
	bOnStreak = false;
	HighestStreak = 0;
	RevengeElims = 0;
	DethroneElims = 0;
	ShowStopperElims = 0;
	bFirstBlood = false;
	bWinner = false;
	bIsProcessingSpecialElimQueue = false;
	ElimDisplayTime = 0.5f;
}

void AShooterPlayerState::AddScoredElims()
{
	++ScoredElims;
}

void AShooterPlayerState::AddDefeats()
{
	++Defeats;
}

void AShooterPlayerState::AddHits()
{
	++Hits;
}

void AShooterPlayerState::AddMisses()
{
	++Misses;
}

void AShooterPlayerState::AddHeadshotElims()
{
	++HeadshotElims;
}

void AShooterPlayerState::AddSequentialElims(int32 SequenceCount)
{
	if (SequentialElims.Contains(SequenceCount))
	{
		++SequentialElims[SequenceCount];
	}
	else
	{
		SequentialElims.Add(SequenceCount, 1);
	}
	
	for (auto& Elem : SequentialElims)
	{
		if (Elem.Key < SequenceCount && Elem.Value > 0)
		{
			Elem.Value--;
		}
	}
}

void AShooterPlayerState::UpdateHighestStreak(int32 StreakCount)
{
	if (StreakCount > HighestStreak)
	{
		HighestStreak = StreakCount;
	}
}

void AShooterPlayerState::AddRevengeElims()
{
	++RevengeElims;
}

void AShooterPlayerState::AddDethroneElims()
{
	++DethroneElims;
}

void AShooterPlayerState::AddShowStopperElims()
{
	++ShowStopperElims;
}

void AShooterPlayerState::GotFirstBlood()
{
	bFirstBlood = true;
}

void AShooterPlayerState::SetWinner()
{
	bWinner = true;
}

TArray<ESpecialElimType> AShooterPlayerState::DecodeElimBitmask(ESpecialElimType ElimType)
{
	TArray<ESpecialElimType> DecodedElims;
	uint16 Bitmask = static_cast<uint16>(ElimType);
	for (int16 i = 0; i < 16; ++i)
	{
		if (Bitmask & (1 << i))
		{
			DecodedElims.Add(static_cast<ESpecialElimType>(1 << i));
		}
	}
	return DecodedElims;
}

void AShooterPlayerState::ProcessNextSpecialElim()
{
	FSpecialElimInfo ElimInfo;
	if (SpecialElimQueue.Dequeue(ElimInfo))
	{
		bIsProcessingSpecialElimQueue = true;
		ShowSpecialElim(ElimInfo);
		
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AShooterPlayerState::ProcessNextSpecialElim, ElimDisplayTime, false);
		});
	}
	else
	{
		bIsProcessingSpecialElimQueue = false;
	}
}

void AShooterPlayerState::ShowSpecialElim(const FSpecialElimInfo& ElimInfo)
{
	FString ElimMessage = ElimInfo.ElimMessage;
	if (ElimInfo.ElimType == ESpecialElimType::Sequential)
	{
		if (ElimInfo.SequentialCount > 1)
		{
			ElimMessage = FString::Printf(TEXT("%s x%d"), *ElimMessage, ElimInfo.SequentialCount);
		}
	}
	else if (ElimInfo.ElimType == ESpecialElimType::Streak)
	{
		if (ElimInfo.StreakCount > 1)
		{
			ElimMessage = FString::Printf(TEXT("%s x%d"), *ElimMessage, ElimInfo.StreakCount);
		}
	}
	
	if (IsValid(SpecialElimWidgetClass))
	{
		USpecialElim* SpecialElimWidget = CreateWidget<USpecialElim>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(SpecialElimWidget))
		{
			SpecialElimWidget->InitializeWidget(FText::FromString(ElimMessage), ElimInfo.ElimIcon);
			SpecialElimWidget->AddToViewport();
		}
	}
}

void AShooterPlayerState::Client_SpecialElim_Implementation(const ESpecialElimType& ElimType, int32 SequentialCount, int32 StreakCount, int32 ElimScore)
{
	ensure(IsValid(SpecialElimData));
	OnScoreChanged.Broadcast(ElimScore);
	TArray<ESpecialElimType> DecodedElims = DecodeElimBitmask(ElimType);
	for (const ESpecialElimType& Type : DecodedElims)
	{
		FSpecialElimInfo& ElimMessageInfo = SpecialElimData->SpecialElimMap.FindChecked(Type);
		if (Type == ESpecialElimType::Sequential)
		{
			ElimMessageInfo.SequentialCount = SequentialCount;
		}
		else if (Type == ESpecialElimType::Streak)
		{
			ElimMessageInfo.StreakCount = StreakCount;
		}
		ElimMessageInfo.ElimType = Type;
		
		SpecialElimQueue.Enqueue(ElimMessageInfo);
	}
	if (!bIsProcessingSpecialElimQueue)
	{
		ProcessNextSpecialElim();
	}
}

void AShooterPlayerState::Client_ScoredElim_Implementation(int32 ElimScore)
{
	OnScoreChanged.Broadcast(ElimScore);
}

void AShooterPlayerState::Client_LostTheLead_Implementation()
{
	ensure(IsValid(SpecialElimData));
	FSpecialElimInfo& ElimMessageInfo = SpecialElimData->SpecialElimMap.FindChecked(ESpecialElimType::LostTheLead);
	
	if (IsValid(SpecialElimWidgetClass))
	{
		USpecialElim* SpecialElimWidget = CreateWidget<USpecialElim>(GetPlayerController(), SpecialElimWidgetClass);
		if (IsValid(SpecialElimWidget))
		{
			SpecialElimWidget->InitializeWidget(FText::FromString(ElimMessageInfo.ElimMessage), ElimMessageInfo.ElimIcon);
			SpecialElimWidget->CenterWidget(SpecialElimWidget, 0.5f);
			SpecialElimWidget->AddToViewport();
		}
	}
}
