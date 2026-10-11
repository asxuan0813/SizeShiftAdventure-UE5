#include "SizeShiftPromptWidget.h"
#include "Components/TextBlock.h"

void USizeShiftPromptWidget::SetPromptState(
    bool showInteract,
    bool showSizeUp,
    bool showSizeDown)
{
    if (_InteractText)
    {
        _InteractText->SetVisibility(
            showInteract
            ? ESlateVisibility::Visible
            : ESlateVisibility::Collapsed
        );
    }

    if (_SizeUpText)
    {
        _SizeUpText->SetVisibility(
            showSizeUp
            ? ESlateVisibility::Visible
            : ESlateVisibility::Collapsed
        );
    }

    if (_SizeDownText)
    {
        _SizeDownText->SetVisibility(
            showSizeDown
            ? ESlateVisibility::Visible
            : ESlateVisibility::Collapsed
        );
    }
}

void USizeShiftPromptWidget::SetPromptPosition(
    FVector2D screenPosition)
{
    SetAlignmentInViewport(
        FVector2D(0.5f, 0.5f)
    );

    SetPositionInViewport(
        screenPosition,
        true
    );
}