#include "SizeShiftPromptWidget.h"
#include "Components/TextBlock.h"

void USizeShiftPromptWidget::SetPromptState(
    bool bShowInteract,
    bool bShowSizeUp,
    bool bShowSizeDown)
{
    if (InteractText)
    {
        InteractText->SetVisibility(
            bShowInteract
            ? ESlateVisibility::Visible
            : ESlateVisibility::Collapsed
        );
    }

    if (SizeUpText)
    {
        SizeUpText->SetVisibility(
            bShowSizeUp
            ? ESlateVisibility::Visible
            : ESlateVisibility::Collapsed
        );
    }

    if (SizeDownText)
    {
        SizeDownText->SetVisibility(
            bShowSizeDown
            ? ESlateVisibility::Visible
            : ESlateVisibility::Collapsed
        );
    }
}

void USizeShiftPromptWidget::SetPromptPosition(
    FVector2D ScreenPosition)
{
    SetPositionInViewport(
        ScreenPosition,
        false
    );
}