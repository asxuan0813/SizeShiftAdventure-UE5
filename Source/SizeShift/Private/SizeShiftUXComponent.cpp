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

	if (_PromptWidgetClass)
    {
        _PromptWidget = CreateWidget<USizeShiftPromptWidget>(
            GetWorld(),
            _PromptWidgetClass
        );

        if (_PromptWidget)
        {
            _PromptWidget->AddToViewport();

            _PromptWidget->SetPromptState(
                false,
                false,
                false
            );
        }
    }	
}

void USizeShiftUXComponent::UpdateTargetOutline(
    ASizeShiftBox* newTarget,
    bool canInteract,
    bool canSizeShift,
    bool isHolding)
{
    if (!newTarget)
    {
        ClearCurrentOutline();
        return;
    }

    UMaterialInterface* desiredMaterial = nullptr;

    // Highest priority: Holding
    if (isHolding)
    {
        desiredMaterial = _HoldingOutlineMaterial;
    }
    // Can interact
    else if (canInteract)
    {
        desiredMaterial = _InteractOutlineMaterial;
    }
    // Can size shift
    else if (canSizeShift)
    {
        desiredMaterial = _SizeShiftOutlineMaterial;
    }
    // Normal target
    else
    {
        desiredMaterial = _NormalOutlineMaterial;
    }

    if (_CurrentOutlineTarget != newTarget)
    {
        ClearCurrentOutline();

        _CurrentOutlineTarget = newTarget;
    }

    ApplyOutlineMaterial(desiredMaterial);
}

void USizeShiftUXComponent::ApplyOutlineMaterial(
    UMaterialInterface* newMaterial)
{
    if (!_CurrentOutlineTarget)
    {
        return;
    }

    UStaticMeshComponent* boxMesh =
        _CurrentOutlineTarget->GetBoxMesh();

    if (!boxMesh)
    {
        return;
    }

    boxMesh->SetOverlayMaterial(newMaterial);
}

void USizeShiftUXComponent::ClearCurrentOutline()
{
    if (!_CurrentOutlineTarget)
    {
        return;
    }

    if (UStaticMeshComponent* boxMesh =
        _CurrentOutlineTarget->GetBoxMesh())
    {
        boxMesh->SetOverlayMaterial(nullptr);
    }

    _CurrentOutlineTarget = nullptr;
}

void USizeShiftUXComponent::SetPromptState(
    bool showInteract,
    bool showSizeUp,
    bool showSizeDown)
{
    if (!_PromptWidget)
    {
        return;
    }

    _PromptWidget->SetPromptState(
        showInteract,
        showSizeUp,
        showSizeDown
    );
}

void USizeShiftUXComponent::SetPromptPosition(
    FVector2D screenPosition)
{
    if (!_PromptWidget)
    {
        return;
    }

    _PromptWidget->SetPromptPosition(screenPosition);
}

void USizeShiftUXComponent::PlaySizeShiftSound()
{
    if (!_SizeShiftSound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        _SizeShiftSound
    );
}

void USizeShiftUXComponent::PlayInteractSound()
{
    if (!_InteractSound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        _InteractSound
    );
}

void USizeShiftUXComponent::PlayThrowSound()
{
    if (!_ThrowSound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        _ThrowSound
    );
}

void USizeShiftUXComponent::PlayInvalidActionSound()
{
    if (!_InvalidActionSound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        _InvalidActionSound
    );
}

void USizeShiftUXComponent::PlayDeathSound()
{
    if (!_DeathSound)
    {
        return;
    }

    UGameplayStatics::PlaySound2D(
        GetWorld(),
        _DeathSound
    );
}


