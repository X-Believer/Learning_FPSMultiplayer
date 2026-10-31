// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreWidget.generated.h"

class UTextBlock;
class AShooterPlayerState;
/**
 * 
 */
UCLASS()
class FPS_API UScoreWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Score;
	
private:
	AShooterPlayerState* GetShooterPlayerState() const;
	
	UFUNCTION()
	void OnScoreChanged(int32 NewScore);
	
	UFUNCTION()
	void OnPlayerStateReplicated();
};
