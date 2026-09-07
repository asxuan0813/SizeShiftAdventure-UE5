#include "SizeShiftBox.h"

#include "Components/StaticMeshComponent.h"

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

void ASizeShiftBox::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    UpdateBoxProperties();
}

void ASizeShiftBox::BeginPlay()
{
    Super::BeginPlay();

    UpdateBoxProperties();
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

EBoxSizeType ASizeShiftBox::GetBoxSize() const
{
    return SizeType;
}

EBoxInteractionType ASizeShiftBox::GetInteractionType() const
{
    return InteractionType;
}

float ASizeShiftBox::GetDensity() const
{
    return Density;
}

float ASizeShiftBox::GetVolume() const
{
    return Volume;
}

float ASizeShiftBox::GetMass() const
{
    return Mass;
}

float ASizeShiftBox::GetSizeMultiplier() const
{
    switch (SizeType)
    {
    case EBoxSizeType::Small:
        return 0.49f;

    case EBoxSizeType::Medium:
        return 0.99f;

    case EBoxSizeType::Large:
        return 1.99f;

    default:
        return 0.99f;
    }
}

float ASizeShiftBox::ResolveMaterialDensity() const
{
    if (MaterialData)
    {
        return MaterialData->MaterialData.Density;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[SizeShiftBox] No MaterialData assigned. Using default density.")
    );

    return 200.0f;
}

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

void ASizeShiftBox::UpdateBoxProperties()
{
    const float SizeMultiplier =
        GetSizeMultiplier();

    // --------------------------------------------------------
    // Calculate physics values
    // --------------------------------------------------------

    Volume = 
        BaseVolume *
        SizeMultiplier *
        SizeMultiplier *
        SizeMultiplier;

    Density = ResolveMaterialDensity();

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







