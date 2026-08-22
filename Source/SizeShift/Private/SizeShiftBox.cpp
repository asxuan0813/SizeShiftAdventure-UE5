#include "SizeShiftBox.h"

#include "Components/StaticMeshComponent.h"

// ============================================================
// Constructor
// ============================================================

ASizeShiftBox::ASizeShiftBox()
{
    PrimaryActorTick.bCanEverTick = false;

    BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("BoxMesh")
    );

    RootComponent = BoxMesh;

    BoxMesh->SetSimulatePhysics(true);
    BoxMesh->SetEnableGravity(true);

    BoxMesh->SetCollisionEnabled(
        ECollisionEnabled::QueryAndPhysics
    );
}

// ============================================================
// Size
// ============================================================

float ASizeShiftBox::GetSizeMultiplier() const
{
    switch (SizeType)
    {
    case EBoxSizeType::Small:
        return 0.5f;

    case EBoxSizeType::Medium:
        return 1.0f;

    case EBoxSizeType::Large:
        return 1.5f;

    default:
        return 1.0f;
    }
}

void ASizeShiftBox::IncreaseBoxSize()
{
    switch (SizeType)
    {
    case EBoxSizeType::Small:

        SetBoxSize(EBoxSizeType::Medium);

        break;

    case EBoxSizeType::Medium:

        SetBoxSize(EBoxSizeType::Large);

        break;

    case EBoxSizeType::Large:

        PrintBoxStatus();

        break;

    default:

        break;
    }
}

void ASizeShiftBox::DecreaseBoxSize()
{
    switch (SizeType)
    {
    case EBoxSizeType::Small:

        PrintBoxStatus();

        break;

    case EBoxSizeType::Medium:

        SetBoxSize(EBoxSizeType::Small);

        break;

    case EBoxSizeType::Large:

        SetBoxSize(EBoxSizeType::Medium);

        break;

    default:

        break;
    }
}

void ASizeShiftBox::SetBoxSize(EBoxSizeType NewSize)
{
    if (SizeType == NewSize)
    {
        return;
    }

    SizeType = NewSize;

    UpdateBoxProperties();
}

EBoxSizeType ASizeShiftBox::GetBoxSize() const
{
    return SizeType;
}

// ============================================================
// Material
// ============================================================

float ASizeShiftBox::GetDensity() const
{
    if (!MaterialData)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[SizeShiftBox] No MaterialData assigned!")
        );

        return 200.0f;
    }

    return MaterialData->MaterialData.Density;
}

// ============================================================
// Interaction
// ============================================================

void ASizeShiftBox::UpdateInteractionType()
{
    switch (SizeType)
    {
    case EBoxSizeType::Small:

        InteractionType =
            EBoxInteractionType::PickupThrow;

        break;

    case EBoxSizeType::Medium:

        InteractionType =
            EBoxInteractionType::PushPull;

        break;

    case EBoxSizeType::Large:

        InteractionType =
            EBoxInteractionType::Immovable;

        break;

    default:

        InteractionType =
            EBoxInteractionType::Immovable;

        break;
    }
}

EBoxInteractionType ASizeShiftBox::GetInteractionType() const
{
    return InteractionType;
}

// ============================================================
// Physics
// ============================================================

float ASizeShiftBox::GetVolume() const
{
    const float SizeMultiplier =
        GetSizeMultiplier();

    return BaseVolume *
        SizeMultiplier *
        SizeMultiplier *
        SizeMultiplier;
}

float ASizeShiftBox::GetMass() const
{
    return GetVolume() * GetDensity();
}

void ASizeShiftBox::UpdateBoxProperties()
{
    const float SizeMultiplier =
        GetSizeMultiplier();

    // --------------------------------------------------------
    // Calculate physics values
    // --------------------------------------------------------

    Volume = GetVolume();

    Density = GetDensity();

    Mass = Volume * Density;

    // --------------------------------------------------------
    // Update visual size
    // --------------------------------------------------------

    SetActorScale3D(
        FVector(SizeMultiplier)
    );

    // --------------------------------------------------------
    // Update physics mass
    // --------------------------------------------------------

    if (BoxMesh)
    {
        BoxMesh->SetMassOverrideInKg(
            NAME_None,
            Mass,
            true
        );

        BoxMesh->WakeAllRigidBodies();
    }

    // --------------------------------------------------------
    // Size determines Interaction
    // --------------------------------------------------------

    UpdateInteractionType();

    // --------------------------------------------------------
    // Debug
    // --------------------------------------------------------

    PrintBoxStatus();
}

// ============================================================
// Begin Play
// ============================================================

void ASizeShiftBox::BeginPlay()
{
    Super::BeginPlay();

    UpdateBoxProperties();
}

// ============================================================
// Debug
// ============================================================

void ASizeShiftBox::PrintBoxStatus() const
{
    FString SizeName;
    FString InteractionName;

    switch (SizeType)
    {
    case EBoxSizeType::Small:
        SizeName = TEXT("Small");
        break;

    case EBoxSizeType::Medium:
        SizeName = TEXT("Medium");
        break;

    case EBoxSizeType::Large:
        SizeName = TEXT("Large");
        break;

    default:
        SizeName = TEXT("Unknown");
        break;
    }

    switch (InteractionType)
    {
    case EBoxInteractionType::PickupThrow:
        InteractionName = TEXT("Pickup / Throw");
        break;

    case EBoxInteractionType::PushPull:
        InteractionName = TEXT("Push / Pull");
        break;

    case EBoxInteractionType::Immovable:
        InteractionName = TEXT("Immovable");
        break;

    default:
        InteractionName = TEXT("Unknown");
        break;
    }

    FString MaterialName = TEXT("None");

    if (MaterialData)
    {
        MaterialName =
            MaterialData->MaterialName.ToString();
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[SizeShiftBox] Material: %s | Size: %s | Interaction: %s | Scale: %.2f | Volume: %.3f | Density: %.3f | Mass: %.3f kg"
        ),
        *MaterialName,
        *SizeName,
        *InteractionName,
        GetSizeMultiplier(),
        Volume,
        Density,
        Mass
    );
}
