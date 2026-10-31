// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpecialElim.generated.h"


class UImage;
class UTextBlock;

UCLASS()
class FPS_API USpecialElim : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_ElimMessage;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_ElimIcon;
	
	void InitializeWidget(const FText& ElimMessage, UTexture2D* ElimIcon);
	
	UFUNCTION(BlueprintCallable)
	static void CenterWidget(UUserWidget* Widget, float VerticalRatio = 0.f);
};
