#include "SizeShiftBox.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "CollisionQueryParams.h"

namespace
{
    constexpr float BaseHalfSize = 50.0f;

    constexpr ECollisionChannel SizeShiftBlocker =
        ECC_GameTraceChannel1;
}

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

    const EBoxSizeType OldSize = SizeType;

    SizeType = NewSize;

    UpdateBoxProperties();

    if (GetSizeMultiplier(NewSize) > GetSizeMultiplier(OldSize))
    {
        ApplyGrowImpact();
    }
}

void ASizeShiftBox::IncreaseBoxSize()
{
    switch (SizeType)
    {
    case EBoxSizeType::Small:

        if (!CanResizeTo(EBoxSizeType::Medium))
        {
            return;
        }

        SetBoxSize(EBoxSizeType::Medium);
        break;

    case EBoxSizeType::Medium:

        if (!CanResizeTo(EBoxSizeType::Large))
        {
            return;
        }

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

bool ASizeShiftBox::HasPushPullGroundSupport() const
{
    if (!GetWorld() || !BoxMesh)
    {
        return false;
    }

    const FVector BoxLocation =
        BoxMesh->GetComponentLocation();

    const FQuat BoxRotation =
        BoxMesh->GetComponentQuat();

    const FVector BoxExtent =
        BoxMesh->Bounds.BoxExtent;

    // Start slightly inside the box.
    // End slightly below the box.
    const FVector TraceStart =
        BoxLocation;

    const FVector TraceEnd =
        BoxLocation - FVector::UpVector * 8.0f;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    FHitResult Hit;

    const bool bHit =
        GetWorld()->SweepSingleByChannel(
            Hit,
            TraceStart,
            TraceEnd,
            BoxRotation,
            ECC_WorldStatic,
            FCollisionShape::MakeBox(BoxExtent),
            QueryParams
        );

    return bHit && Hit.bBlockingHit;
}

bool ASizeShiftBox::CanResizeTo(EBoxSizeType NewSize) const
{
    const float CurrentMultiplier = GetSizeMultiplier(SizeType);
    const float TargetMultiplier = GetSizeMultiplier(NewSize);

    const float ExpansionDistance =
        (TargetMultiplier - CurrentMultiplier) * BaseHalfSize;

    if (ExpansionDistance <= 0.0f)
    {
        return true;
    }

    const bool bCanExpandPositiveX =
        CanResizeInDirection(FVector::ForwardVector, ExpansionDistance);

    const bool bCanExpandNegativeX =
        CanResizeInDirection(-FVector::ForwardVector, ExpansionDistance);

    const bool bCanExpandPositiveY =
        CanResizeInDirection(FVector::RightVector, ExpansionDistance);

    const bool bCanExpandNegativeY =
        CanResizeInDirection(-FVector::RightVector, ExpansionDistance);

    const bool bCanExpandPositiveZ =
        CanResizeInDirection(FVector::UpVector, ExpansionDistance);

    const bool bCanExpandNegativeZ =
        CanResizeInDirection(-FVector::UpVector, ExpansionDistance);

    const bool bXBlocked =
        !bCanExpandPositiveX && !bCanExpandNegativeX;

    const bool bYBlocked =
        !bCanExpandPositiveY && !bCanExpandNegativeY;

    const bool bZBlocked =
        !bCanExpandPositiveZ && !bCanExpandNegativeZ;

    return !bXBlocked && !bYBlocked && !bZBlocked;
}

bool ASizeShiftBox::CanResizeInDirection(
    const FVector& Direction,
    float RequiredDistance
) const
{
    if (!GetWorld() || !BoxMesh || RequiredDistance <= 0.0f)
    {
        return true;
    }

    const FVector WorldDirection = Direction.GetSafeNormal();

    if (WorldDirection.IsNearlyZero())
    {
        return true;
    }

    const float CurrentHalfSize =
        BaseHalfSize * GetSizeMultiplier(SizeType);

    const FVector LocalDirection =
        GetActorQuat().UnrotateVector(WorldDirection).GetSafeNormal();

    FVector ExpansionExtent(CurrentHalfSize);

    if (!FMath::IsNearlyZero(LocalDirection.X))
    {
        ExpansionExtent.X = RequiredDistance * 0.5f;
    }

    if (!FMath::IsNearlyZero(LocalDirection.Y))
    {
        ExpansionExtent.Y = RequiredDistance * 0.5f;
    }

    if (!FMath::IsNearlyZero(LocalDirection.Z))
    {
        ExpansionExtent.Z = RequiredDistance * 0.5f;
    }

    const FVector LocalOffset =
        LocalDirection *
        (CurrentHalfSize + RequiredDistance * 0.5f);

    const FVector WorldCenter =
        GetActorLocation() +
        GetActorQuat().RotateVector(LocalOffset);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    const bool bBlocked =
        GetWorld()->OverlapAnyTestByChannel(
            WorldCenter,
            GetActorQuat(),
            SizeShiftBlocker,
            FCollisionShape::MakeBox(ExpansionExtent),
            QueryParams
        );

    return !bBlocked;
}

void ASizeShiftBox::ApplyGrowImpact()
{
    if (!GetWorld())
    {
        return;
    }

    constexpr float ImpactRadius = 150.0f;

    TArray<FOverlapResult> Overlaps;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    const bool bFoundPlayer =
        GetWorld()->OverlapMultiByChannel(
            Overlaps,
            GetActorLocation(),
            FQuat::Identity,
            ECC_Pawn,
            FCollisionShape::MakeSphere(ImpactRadius),
            QueryParams
        );

    if (!bFoundPlayer)
    {
        return;
    }

    for (const FOverlapResult& Overlap : Overlaps)
    {
        ACharacter* OtherCharacter =
            Cast<ACharacter>(Overlap.GetActor());

        if (!OtherCharacter)
        {
            continue;
        }

        FVector KnockbackDirection =
            OtherCharacter->GetActorLocation() - GetActorLocation();

        KnockbackDirection.Normalize();

        float HorizontalImpulse = 0.0f;
        float VerticalImpulse = 0.0f;

        if (SizeType == EBoxSizeType::Medium)
        {
            HorizontalImpulse = SmallToMediumHorizontalImpulse;
            VerticalImpulse = SmallToMediumVerticalImpulse;
        }
        else if (SizeType == EBoxSizeType::Large)
        {
            HorizontalImpulse = MediumToLargeHorizontalImpulse;
            VerticalImpulse = MediumToLargeVerticalImpulse;
        }

        const FVector Impulse =
            KnockbackDirection * HorizontalImpulse
            + FVector::UpVector * VerticalImpulse;

        OtherCharacter->LaunchCharacter(
            Impulse,
            false,
            false
        );
    }
}

float ASizeShiftBox::GetSizeMultiplier() const
{
    return GetSizeMultiplier(SizeType);
}

float ASizeShiftBox::GetSizeMultiplier(EBoxSizeType InSize) const
{
    switch (InSize)
    {
    case EBoxSizeType::Small:
        return 0.49f;

    case EBoxSizeType::Medium:
        return 0.99f;

    case EBoxSizeType::Large:
        return 1.99f;

    default:
        return 1.0f;
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







