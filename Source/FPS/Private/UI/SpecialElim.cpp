// Shuyang Xing All Rights Deserve


#include "UI/SpecialElim.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void USpecialElim::InitializeWidget(const FText& ElimMessage, UTexture2D* ElimIcon)
{
	if (IsValid(Text_ElimMessage))
	{
		Text_ElimMessage->SetText(ElimMessage);
	}
	if (IsValid(Image_ElimIcon))
	{
		Image_ElimIcon->SetBrushFromTexture(ElimIcon);
	}
}

void USpecialElim::CenterWidget(UUserWidget* Widget, float VerticalRatio)
{
	if (!IsValid(Widget)) return;
	
	FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(Widget);
	const float VerticalFraction = VerticalRatio == 0.f ? 1.f : VerticalRatio * 2.f;
	FVector2D CenterPosition = FVector2D(ViewportSize.X * 0.5f, ViewportSize.Y * VerticalFraction * 0.5f);
	Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	Widget->SetPositionInViewport(CenterPosition, false);
}
