#include "SizeShiftUXComponent.h"
#include "SizeShiftBox.h"
#include "Kismet/GameplayStatics.h"
#include "SizeShiftPromptWidget.h"

USizeShiftUXComponent::USizeShiftUXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void USizeShiftUXComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PromptWidgetClass)
    {
        PromptWidget = CreateWidget<USizeShiftPromptWidget>(
            GetWorld(),
            PromptWidgetClass
        );

        if (PromptWidget)
        {
            PromptWidget->AddToViewport();

            PromptWidget->SetPromptState(
                false,
                false,
                false
            );
        }
    }	
}

void USizeShiftUXComponent::UpdateTargetOutline(
    ASizeShiftBox* NewTarget,
    bool bCanInteract,
    bool bCanSizeShift,
    bool bIsHolding)
{
    if (!NewTarget)
    {
        ClearCurrentOutline();
        return;
    }

    UMaterialInterface* DesiredMaterial = nullptr;

    // Highest priority: Holding
    if (bIsHolding)
    {
        DesiredMaterial = HoldingOutlineMaterial;
    }
    // Can interact
    else if (bCanInteract)
    {
        DesiredMaterial = InteractOutlineMaterial;
    }
    // Can size shift
    else if (bCanSizeShift)
    {
        DesiredMaterial = SizeShiftOutlineMaterial;
    }
    // Normal target
    else
    {
        DesiredMaterial = NormalOutlineMaterial;
    }

    if (CurrentOutlineTarget != NewTarget)
    {
        ClearCurrentOutline();

        CurrentOutlineTarget = NewTarget;
    }

    ApplyOutlineMaterial(DesiredMaterial);
}

void USizeShiftUXComponent::ApplyOutlineMaterial(
    UMaterialInterface* NewMaterial)
{
    if (!CurrentOutlineTarget)
    {
        return;
    }

    UStaticMeshComponent* BoxMesh =
        CurrentOutlineTarget->GetBoxMesh();

    if (!BoxMesh)
    {
        return;
    }

    BoxMesh->SetOverlayMaterial(NewMaterial);
}

void USizeShiftUXComponent::ClearCurrentOutline()
{
    if (!CurrentOutlineTarget)
    {
        return;
    }

    if (UStaticMeshComponent* BoxMesh =
        CurrentOutlineTarget->GetBoxMesh())
    {
        BoxMesh->SetOverlayMaterial(nullptr);
    }

    CurrentOutlineTarget = nullptr;
}

void USizeShiftUXComponent::SetPromptState(
    bool bShowInteract,
    bool bShowSizeUp,
    bool bShowSizeDown)
{
    if (!PromptWidget)
    {
        return;
    }

    PromptWidget->SetPromptState(
        bShowInteract,
        bShowSizeUp,
        bShowSizeDown
    );
}

void USizeShiftUXComponent::SetPromptPosition(
    FVector2D ScreenPosition)
{
    if (!PromptWidget)
    {
        return;
    }

    PromptWidget->SetPromptPosition(ScreenPosition);
}

void USizeShiftUXComponent::PlaySizeShiftSound()
{
    if (!SizeShiftSound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        SizeShiftSound
    );
}

void USizeShiftUXComponent::PlayInteractSound()
{
    if (!InteractSound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        InteractSound
    );
}

void USizeShiftUXComponent::PlayThrowSound()
{
    if (!ThrowSound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        ThrowSound
    );
}

void USizeShiftUXComponent::PlayInvalidActionSound()
{
    if (!InvalidActionSound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        InvalidActionSound
    );
}


