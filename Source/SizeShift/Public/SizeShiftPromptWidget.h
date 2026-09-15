#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SizeShiftPromptWidget.generated.h"

class UTextBlock;

UCLASS()
class SIZESHIFT_API USizeShiftPromptWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Prompt")
    void SetPromptState(
        bool bShowInteract,
        bool bShowSizeUp,
        bool bShowSizeDown
    );

    UFUNCTION(BlueprintCallable, Category = "Prompt")
    void SetPromptPosition(FVector2D ScreenPosition);

protected:

    UPROPERTY(meta = (BindWidget))
    UTextBlock* InteractText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SizeUpText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SizeDownText;
};