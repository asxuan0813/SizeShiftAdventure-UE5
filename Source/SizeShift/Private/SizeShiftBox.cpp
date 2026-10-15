#include "SizeShiftBox.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "CollisionQueryParams.h"

namespace
{
    // Half-size of a Medium box before any size multiplier is applied
    constexpr float BaseHalfSize = 50.0f;

    // Collision channel used to detect obstacles that would block box expansion
    constexpr ECollisionChannel SizeShiftBlocker =
        ECC_GameTraceChannel1;
}

ASizeShiftBox::ASizeShiftBox()
{
    PrimaryActorTick.bCanEverTick = false;

    _BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("BoxMesh")
    );

    RootComponent = _BoxMesh;

    _BoxMesh->SetSimulatePhysics(true);
    _BoxMesh->SetEnableGravity(true);

    _BoxMesh->SetCollisionEnabled(
        ECollisionEnabled::QueryAndPhysics
    );
}

void ASizeShiftBox::OnConstruction(const FTransform& Transform)
{
    // Recalculates editor-visible properties whenever the actor is constructed
    Super::OnConstruction(Transform);

    UpdateBoxProperties();
}

void ASizeShiftBox::BeginPlay()
{
    // Recalculates runtime properties once the box enters play
    Super::BeginPlay();

    UpdateBoxProperties();
}

void ASizeShiftBox::SetBoxSize(E_BoxSizeType newSize)
{
    // Applies a valid size change, refreshes derived properties, and knocks back nearby characters when growing.
    if (_SizeType == newSize)
    {
        return;
    }

    const E_BoxSizeType oldSize = _SizeType;

    _SizeType = newSize;

    UpdateBoxProperties();

    if (GetSizeMultiplier(newSize) > GetSizeMultiplier(oldSize))
    {
        ApplyGrowImpact();
    }
}

void ASizeShiftBox::IncreaseBoxSize()
{
    // Attempts the next larger size only after checking that the new volume has clearance
    switch (_SizeType)
    {
    case E_BoxSizeType::Small:

        if (!CanResizeTo(E_BoxSizeType::Medium))
        {
            return;
        }

        SetBoxSize(E_BoxSizeType::Medium);
        break;

    case E_BoxSizeType::Medium:

        if (!CanResizeTo(E_BoxSizeType::Large))
        {
            return;
        }

        SetBoxSize(E_BoxSizeType::Large);
        break;

    case E_BoxSizeType::Large:

        PrintBoxStatus();
        break;

    default:

        break;
    }
}

void ASizeShiftBox::DecreaseBoxSize()
{
    // Changes to the next smaller size; Small remains at its minimum size
    switch (_SizeType)
    {
    case E_BoxSizeType::Small:

        PrintBoxStatus();

        break;

    case E_BoxSizeType::Medium:

        SetBoxSize(E_BoxSizeType::Small);

        break;

    case E_BoxSizeType::Large:

        SetBoxSize(E_BoxSizeType::Medium);

        break;

    default:

        break;
    }
}

E_BoxSizeType ASizeShiftBox::GetBoxSize() const
{
    // Returns the current configured size level
    return _SizeType;
}

E_BoxInteractionType ASizeShiftBox::GetInteractionType() const
{
    // Returns the interaction mode derived from the current size
    return _InteractionType;
}

float ASizeShiftBox::GetDensity() const
{
    // Returns the material-derived density cached by UpdateBoxProperties
    return _Density;
}

float ASizeShiftBox::GetVolume() const
{
    // Returns the size-adjusted volume cached by UpdateBoxProperties
    return _Volume;
}

float ASizeShiftBox::GetMass() const
{
    // Returns the calculated physics mass cached by UpdateBoxProperties
    return _Mass;
}

bool ASizeShiftBox::HasPushPullGroundSupport() const
{
    // Sweeps just below the mesh to ensure a push/pull box still has static-world support
    if (!GetWorld() || !_BoxMesh)
    {
        return false;
    }

    const FVector boxLocation =
        _BoxMesh->GetComponentLocation();

    const FQuat boxRotation =
        _BoxMesh->GetComponentQuat();

    const FVector boxExtent =
        _BoxMesh->Bounds.BoxExtent;

    // Start slightly inside the box.
    // End slightly below the box.
    const FVector traceStart =
        boxLocation;

    const FVector traceEnd =
        boxLocation - FVector::UpVector * 8.0f;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    FHitResult hit;

    const bool isHit =
        GetWorld()->SweepSingleByChannel(
            hit,
            traceStart,
            traceEnd,
            boxRotation,
            ECC_WorldStatic,
            FCollisionShape::MakeBox(boxExtent),
            QueryParams
        );

    return isHit && hit.bBlockingHit;
}

bool ASizeShiftBox::CanResizeTo(E_BoxSizeType newSize) const
{
    // Tests all three axes to ensure an expanding box has clearance on at least one side of each axis
    const float currentMultiplier = GetSizeMultiplier(_SizeType);
    const float targetMultiplier = GetSizeMultiplier(newSize);

    const float expansionDistance =
        (targetMultiplier - currentMultiplier) * BaseHalfSize;

    if (expansionDistance <= 0.0f)
    {
        return true;
    }

    const bool canExpandPositiveX =
        CanResizeInDirection(FVector::ForwardVector, expansionDistance);

    const bool canExpandNegativeX =
        CanResizeInDirection(-FVector::ForwardVector, expansionDistance);

    const bool canExpandPositiveY =
        CanResizeInDirection(FVector::RightVector, expansionDistance);

    const bool canExpandNegativeY =
        CanResizeInDirection(-FVector::RightVector, expansionDistance);

    const bool canExpandPositiveZ =
        CanResizeInDirection(FVector::UpVector, expansionDistance);

    const bool canExpandNegativeZ =
        CanResizeInDirection(-FVector::UpVector, expansionDistance);

    const bool xBlocked =
        !canExpandPositiveX && !canExpandNegativeX;

    const bool yBlocked =
        !canExpandPositiveY && !canExpandNegativeY;

    const bool zBlocked =
        !canExpandPositiveZ && !canExpandNegativeZ;

    return !xBlocked && !yBlocked && !zBlocked;
}

bool ASizeShiftBox::CanResizeInDirection(
    const FVector& direction,
    float requiredDistance
) const
{
    // Checks the additional volume that expansion would occupy in one world direction
    if (!GetWorld() || !_BoxMesh || requiredDistance <= 0.0f)
    {
        return true;
    }

    const FVector worldDirection = direction.GetSafeNormal();

    if (worldDirection.IsNearlyZero())
    {
        return true;
    }

    const float currentHalfSize =
        BaseHalfSize * GetSizeMultiplier(_SizeType);

    const FVector localDirection =
        GetActorQuat().UnrotateVector(worldDirection).GetSafeNormal();

    FVector expansionExtent(currentHalfSize);

    if (!FMath::IsNearlyZero(localDirection.X))
    {
        expansionExtent.X = requiredDistance * 0.5f;
    }

    if (!FMath::IsNearlyZero(localDirection.Y))
    {
        expansionExtent.Y = requiredDistance * 0.5f;
    }

    if (!FMath::IsNearlyZero(localDirection.Z))
    {
        expansionExtent.Z = requiredDistance * 0.5f;
    }

    const FVector localOffset =
        localDirection *
        (currentHalfSize + requiredDistance * 0.5f);

    const FVector worldCenter =
        GetActorLocation() +
        GetActorQuat().RotateVector(localOffset);

    FCollisionQueryParams queryParams;
    queryParams.AddIgnoredActor(this);

    const bool isBlocked =
        GetWorld()->OverlapAnyTestByChannel(
            worldCenter,
            GetActorQuat(),
            SizeShiftBlocker,
            FCollisionShape::MakeBox(expansionExtent),
            queryParams
        );

    return !isBlocked;
}

void ASizeShiftBox::ApplyGrowImpact()
{
    // Finds nearby characters and launches them with impulses based on the new size
    if (!GetWorld())
    {
        return;
    }

    constexpr float impactRadius = 150.0f;

    TArray<FOverlapResult> overlaps;

    FCollisionQueryParams queryParams;
    queryParams.AddIgnoredActor(this);

    const bool foundPlayer =
        GetWorld()->OverlapMultiByChannel(
            overlaps,
            GetActorLocation(),
            FQuat::Identity,
            ECC_Pawn,
            FCollisionShape::MakeSphere(impactRadius),
            queryParams
        );

    if (!foundPlayer)
    {
        return;
    }

    for (const FOverlapResult& overlap : overlaps)
    {
        ACharacter* otherCharacter =
            Cast<ACharacter>(overlap.GetActor());

        if (!otherCharacter)
        {
            continue;
        }

        FVector knockbackDirection =
            otherCharacter->GetActorLocation() - GetActorLocation();

        knockbackDirection.Normalize();

        float horizontalImpulse = 0.0f;
        float verticalImpulse = 0.0f;

        if (_SizeType == E_BoxSizeType::Medium)
        {
            horizontalImpulse = _SmallToMediumHorizontalImpulse;
            verticalImpulse = _SmallToMediumVerticalImpulse;
        }
        else if (_SizeType == E_BoxSizeType::Large)
        {
            horizontalImpulse = _MediumToLargeHorizontalImpulse;
            verticalImpulse = _MediumToLargeVerticalImpulse;
        }

        const FVector impulse =
            knockbackDirection * horizontalImpulse
            + FVector::UpVector * verticalImpulse;

        otherCharacter->LaunchCharacter(
            impulse,
            false,
            false
        );
    }
}

float ASizeShiftBox::GetCurrentSizeMultiplier() const
{
    // Looks up the multiplier that corresponds to the current size
    return GetSizeMultiplier(_SizeType);
}

float ASizeShiftBox::GetSizeMultiplier(E_BoxSizeType inSize) const
{
    // Maps Small, Medium, and Large to their relative linear scale values
    switch (inSize)
    {
    case E_BoxSizeType::Small:
        return 0.5f;

    case E_BoxSizeType::Medium:
        return 1.0f;

    case E_BoxSizeType::Large:
        return 2.0f;

    default:
        return 1.0f;
    }
}

float ASizeShiftBox::ResolveMaterialDensity() const
{
    // Reads density from material data or returns the fallback density when none is assigned
    if (_MaterialData)
    {
        return _MaterialData->_MaterialProperties.density;
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[SizeShiftBox] No MaterialData assigned. Using default density.")
    );

    return 500.0f;
}

void ASizeShiftBox::UpdateInteractionType()
{
    // Assigns pickup/throw, push/pull, or immovable behaviour from the current size
    switch (_SizeType)
    {
    case E_BoxSizeType::Small:

        _InteractionType =
            E_BoxInteractionType::PickupThrow;

        break;

    case E_BoxSizeType::Medium:

        _InteractionType =
            E_BoxInteractionType::PushPull;

        break;

    case E_BoxSizeType::Large:

        _InteractionType =
            E_BoxInteractionType::Immovable;

        break;

    default:

        _InteractionType =
            E_BoxInteractionType::Immovable;

        break;
    }
}

void ASizeShiftBox::UpdateBoxProperties()
{
    // Central refresh flow: recalculates physics, applies scale and mass, updates interaction type, then logs status
    const float sizeMultiplier =
        GetCurrentSizeMultiplier();

    // --------------------------------------------------------
    // Calculate physics values
    // --------------------------------------------------------

    _Volume = 
        _BaseVolume *
        sizeMultiplier *
        sizeMultiplier *
        sizeMultiplier;

    _Density = ResolveMaterialDensity();

    _Mass = _Volume * _Density;

    // --------------------------------------------------------
    // Update visual size
    // --------------------------------------------------------

    // Apply a separate fit scale to the actual Box.
    const float fitScaleMultiplier = 0.98f;

    SetActorScale3D(
        FVector(sizeMultiplier * fitScaleMultiplier)
    );

    // --------------------------------------------------------
    // Update physics mass
    // --------------------------------------------------------

    if (_BoxMesh)
    {
        _BoxMesh->SetMassOverrideInKg(
            NAME_None,
            _Mass,
            true
        );

        _BoxMesh->WakeAllRigidBodies();
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
    // Formats the current material, size, interaction, and physics values for the output log
    FString sizeName;
    FString interactionName;

    switch (_SizeType)
    {
    case E_BoxSizeType::Small:
        sizeName = TEXT("Small");
        break;

    case E_BoxSizeType::Medium:
        sizeName = TEXT("Medium");
        break;

    case E_BoxSizeType::Large:
        sizeName = TEXT("Large");
        break;

    default:
        sizeName = TEXT("Unknown");
        break;
    }

    switch (_InteractionType)
    {
    case E_BoxInteractionType::PickupThrow:
        interactionName = TEXT("Pickup / Throw");
        break;

    case E_BoxInteractionType::PushPull:
        interactionName = TEXT("Push / Pull");
        break;

    case E_BoxInteractionType::Immovable:
        interactionName = TEXT("Immovable");
        break;

    default:
        interactionName = TEXT("Unknown");
        break;
    }

    FString materialName = TEXT("None");

    if (_MaterialData)
    {
        materialName =
            _MaterialData->_MaterialName.ToString();
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT(
            "[SizeShiftBox] Material: %s | Size: %s | Interaction: %s | Scale: %.2f | Volume: %.3f | Density: %.3f | Mass: %.3f kg"
        ),
        *materialName,
        *sizeName,
        *interactionName,
        GetCurrentSizeMultiplier(),
        _Volume,
        _Density,
        _Mass
    );
}