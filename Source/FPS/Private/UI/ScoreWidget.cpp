// Shuyang Xing All Rights Deserve


#include "UI/ScoreWidget.h"

#include "Components/TextBlock.h"
#include "Player/ShooterPlayerController.h"
#include "Player/ShooterPlayerState.h"

void UScoreWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	AShooterPlayerState* PS = GetShooterPlayerState();
	if (IsValid(PS))
	{
		PS->OnScoreChanged.AddDynamic(this, &UScoreWidget::OnScoreChanged);
	}
	else
	{
		AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer());
		if (IsValid(PC))
		{
			PC->OnPlayerStateReplicated.AddUniqueDynamic(this, &UScoreWidget::OnPlayerStateReplicated);
		}
	}
}

AShooterPlayerState* UScoreWidget::GetShooterPlayerState() const
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) return nullptr;
	return PC->GetPlayerState<AShooterPlayerState>();
}

void UScoreWidget::OnScoreChanged(const int32 NewScore)
{
	if (IsValid(Text_Score))
	{
		Text_Score->SetText(FText::AsNumber(NewScore));
	}
}

void UScoreWidget::OnPlayerStateReplicated()
{
	AShooterPlayerState* PS = GetShooterPlayerState();
	if (IsValid(PS))
	{
		PS->OnScoreChanged.AddDynamic(this, &UScoreWidget::OnScoreChanged);
		OnScoreChanged(PS->GetScoredElims());
	}
	
	AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetOwningPlayer());
	if (IsValid(PC))
	{
		PC->OnPlayerStateReplicated.RemoveDynamic(this, &UScoreWidget::OnPlayerStateReplicated);
	}
}
