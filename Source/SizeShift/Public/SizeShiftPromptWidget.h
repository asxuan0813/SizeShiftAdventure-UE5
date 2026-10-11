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
        bool showInteract,
        bool showSizeUp,
        bool showSizeDown
    );

    UFUNCTION(BlueprintCallable, Category = "Prompt")
    void SetPromptPosition(FVector2D screenPosition);

protected:

    UPROPERTY(meta = (BindWidget))
    UTextBlock* _InteractText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* _SizeUpText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* _SizeDownText;
};