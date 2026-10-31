// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "Data/SpecialElimData.h"
#include "GameFramework/PlayerState.h"
#include "ShooterTypes/ShooterTypes.h"
#include "ShooterPlayerState.generated.h"

class USpecialElim;
class USpecialElimData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreChanged, int32, NewScore);

/**
 * 
 */
UCLASS()
class FPS_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	AShooterPlayerState();
	
	void AddScoredElims();
	void AddDefeats();
	void AddHits();
	void AddMisses();
	void AddHeadshotElims();
	void AddSequentialElims(int32 SequenceCount);
	void UpdateHighestStreak(int32 StreakCount);
	void AddRevengeElims();
	void AddDethroneElims();
	void AddShowStopperElims();
	void GotFirstBlood();
	void SetWinner();
	void SetOnStreak(bool bStreak) { bOnStreak = bStreak; }
	bool IsOnStreak() const { return bOnStreak; }
	void SetLastAttacker(APlayerState* Attacker) { LastAttacker = Attacker; }
	APlayerState* GetLastAttacker() const { return LastAttacker.Get(); }
	int32 GetScoredElims() const { return ScoredElims; }
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|SpecialElim")
	TObjectPtr<USpecialElimData> SpecialElimData;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|SpecialElim")
	TSubclassOf<USpecialElim> SpecialElimWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|SpecialElim")
	float ElimDisplayTime;
	
	UPROPERTY(BlueprintAssignable)
	FScoreChanged OnScoreChanged;
	
	UFUNCTION(Client, Reliable)
	void Client_LostTheLead();
	
	UFUNCTION(Client, Reliable)
	void Client_ScoredElim(int32 ElimScore);
	
	UFUNCTION(Client, Reliable)
	void Client_SpecialElim(const ESpecialElimType& ElimType, int32 SequentialCount, int32 StreakCount, int32 ElimScore);
	
private:
	int32 ScoredElims;
	int32 Defeats;
	int32 Hits;
	int32 Misses;
	int32 HeadshotElims;
	bool bOnStreak;
	TMap<int32, int32> SequentialElims;
	int32 HighestStreak;
	int32 RevengeElims;
	int32 DethroneElims;
	int32 ShowStopperElims;
	bool bFirstBlood;
	bool bWinner;
	TWeakObjectPtr<APlayerState> LastAttacker;
	
	TArray<ESpecialElimType> DecodeElimBitmask(ESpecialElimType ElimType);
	TQueue<FSpecialElimInfo> SpecialElimQueue;
	bool bIsProcessingSpecialElimQueue;
	
	void ProcessNextSpecialElim();
	void ShowSpecialElim(const FSpecialElimInfo& ElimInfo);
};
