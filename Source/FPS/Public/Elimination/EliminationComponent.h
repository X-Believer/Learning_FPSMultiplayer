// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterTypes/ShooterTypes.h"
#include "EliminationComponent.generated.h"


class AShooterGameStateBase;
class AShooterPlayerState;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UEliminationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEliminationComponent();

	UPROPERTY(EditDefaultsOnly, Category="FPS|Elimination")
	int32 ElimNeededForStreak;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Elimination")
	float SequentialElimInterval;
	
	UFUNCTION()
	void OnRoundReported(AActor* Victim, AActor* Attacker, bool bHit, bool bHeadshot, bool bLethal);
	
private:
	
	float LastElimTime;
	int32 SequentialElimCount;
	int32 StreakCount;
	
	AShooterPlayerState* GetPlayerStateFromActor(AActor* Actor);
	void ProcessHitOrMiss(bool bHit, AShooterPlayerState* AttackerPlayerState);
	void ProcessElimination(bool bHeadshot, AShooterPlayerState* VictimPlayerState, AShooterPlayerState* AttackerPlayerState);
	void ProcessHeadshot(bool bHeadshot, ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS);
	void ProcessSequentialElim(ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS);
	void ProcessStreak(ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS, AShooterPlayerState* VictimPS);
	void HandleFirstBlood(AShooterGameStateBase* GameState, ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS);
	void UpdateLeaderStatus(AShooterGameStateBase* GameState, ESpecialElimType& OutElimType, AShooterPlayerState* AttackerPS, AShooterPlayerState* VictimPS);
	bool HasSpecialElim(const ESpecialElimType& ElimType);
};