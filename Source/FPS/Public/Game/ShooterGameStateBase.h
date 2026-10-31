// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterGameStateBase.generated.h"

class AShooterPlayerState;
/**
 * 
 */
UCLASS()
class FPS_API AShooterGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AShooterGameStateBase();
	
	bool HasFirstBloodOccurred() const { return bHasFirstBloodOccurred; }
	void UpdateLeader();
	AShooterPlayerState* GetSoleLeader();
	bool IsTiedForTheLead(AShooterPlayerState* PlayerState);
	
private:
	bool bHasFirstBloodOccurred;
	
	UPROPERTY()
	TArray<TObjectPtr<AShooterPlayerState>> Leaders;
};
