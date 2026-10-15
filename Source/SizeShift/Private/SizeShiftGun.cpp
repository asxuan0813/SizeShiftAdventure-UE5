// Fill out your copyright notice in the Description page of Project Settings.

#include "SizeShiftGun.h"
#include "SizeShiftUXComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "SizeShiftBox.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SizeShiftCharacter.h"

ASizeShiftGun::ASizeShiftGun()
{
    PrimaryActorTick.bCanEverTick = true;

    _GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("GunMesh")
    );

    RootComponent = _GunMesh;

    _UXComponent = CreateDefaultSubobject<USizeShiftUXComponent>(
        TEXT("UXComponent")
    );

    _HeldBoxPreview =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("HeldBoxPreview")
        );

    _HeldBoxPreview->SetupAttachment(_GunMesh);

    _HeldBoxPreview->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    _HeldBoxPreview->SetSimulatePhysics(false);

    _HeldBoxPreview->SetCastShadow(false);

    _HeldBoxPreview->SetRenderCustomDepth(true);

    _HeldBoxPreview->SetCustomDepthStencilValue(1);

    _HeldBoxPreview->SetVisibility(false);
    
    _CurrentAimTarget = nullptr;
    _ActiveInteractionTarget = nullptr;

    _GunState = E_SizeShiftGunState::Idle;
}

void ASizeShiftGun::BeginPlay()
{
    // Sets the initial runtime state and remembers the default hold distance
	Super::BeginPlay();

    SetGunState(
        E_SizeShiftGunState::Idle
    );

    _DefaultHoldDistance = _HoldDistance;
	
}

void ASizeShiftGun::Tick(float DeltaTime)
{
    // Updates aim detection, then advances any active pickup or push/pull behaviour.
    Super::Tick(DeltaTime);

    UpdateAimTarget();

    UpdatePickupThrow();

    UpdatePushPull(DeltaTime);
}

ASizeShiftBox* ASizeShiftGun::GetAimTarget() const
{
    // Traces from the player's camera and returns a SizeShiftBox hit by the visibility channel.
    APlayerController* playerController =
        GetPlayerController();

    if (!playerController)
    {
        return nullptr;
    }

    FVector cameraLocation;
    FRotator cameraRotation;

    playerController->GetPlayerViewPoint(
        cameraLocation,
        cameraRotation
    );

    const FVector start = cameraLocation;

    const FVector end =
        start +
        cameraRotation.Vector() * _TargetTraceDistance;

    FHitResult hit;

    FCollisionQueryParams params;
    params.AddIgnoredActor(this);
    params.AddIgnoredActor(GetOwner());

    const bool isHit =
        GetWorld()->LineTraceSingleByChannel(
            hit,
            start,
            end,
            ECC_Visibility,
            params
        );

    if (!isHit)
    {
        return nullptr;
    }

    return Cast<ASizeShiftBox>(hit.GetActor());
}

void ASizeShiftGun::UpdateAimTarget()
{
    // Stores aim-target changes and refreshes the gun UI every frame.
    ASizeShiftBox* newAimTarget = GetAimTarget();

    if (newAimTarget != _CurrentAimTarget)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Aim Debug] Target Changed: %s -> %s"),
            IsValid(_CurrentAimTarget)
            ? *_CurrentAimTarget->GetName()
            : TEXT("None"),
            IsValid(newAimTarget)
            ? *newAimTarget->GetName()
            : TEXT("None")
        );

        _CurrentAimTarget = newAimTarget;
    }

    RefreshUX();
}

void ASizeShiftGun::IncreaseSize()
{
    // Increase a valid target, then plays success or invalid-action feedback
    if (!CanCurrentlySizeShift())
    {
        return;
    }

    if (_GunState == E_SizeShiftGunState::Interacting)
    {
        PrintGunStatus(
            TEXT("INCREASE SIZE BLOCKED -> Interacting")
        );

        return;
    }

    const E_BoxSizeType oldSize =
        _CurrentAimTarget->GetBoxSize();

    _CurrentAimTarget->IncreaseBoxSize();

    const E_BoxSizeType newSize =
        _CurrentAimTarget->GetBoxSize();

    if (_UXComponent)
    {
        if (oldSize != newSize)
        {
            _UXComponent->PlaySizeShiftSound();
        }
        else
        {
            _UXComponent->PlayInvalidActionSound();
        }
    }

    PrintGunStatus(
        FString::Printf(
            TEXT("SIZE UP -> %s"),
            *_CurrentAimTarget->GetName()
        )
    );
}

void ASizeShiftGun::DecreaseSize()
{
    // Decrease a valid target, with feedback when its size cannot change further
    if (!CanCurrentlySizeShift())
    {
        return;
    }

    if (_GunState == E_SizeShiftGunState::Interacting)
    {
        PrintGunStatus(
            TEXT("DECREASE SIZE BLOCKED -> Interacting")
        );

        return;
    }

    const E_BoxSizeType oldSize =
        _CurrentAimTarget->GetBoxSize();

    _CurrentAimTarget->DecreaseBoxSize();

    const E_BoxSizeType newSize =
        _CurrentAimTarget->GetBoxSize();

    if (oldSize != newSize)
    {
        if (_UXComponent)
        {
            _UXComponent->PlaySizeShiftSound();
        }
    }
    else if (oldSize == E_BoxSizeType::Small)
    {
        if (_UXComponent)
        {
            _UXComponent->PlayInvalidActionSound();
        }
    }

    PrintGunStatus(
        FString::Printf(
            TEXT("SIZE DOWN -> %s"),
            *_CurrentAimTarget->GetName()
        )
    );
}

void ASizeShiftGun::UnlockSizeShift()
{
    // Enables size shifting and immediately refreshes the target outline and prompts
    _CanSizeShift = true;

    if (_UXComponent)
    {
        _UXComponent->UpdateTargetOutline(
            _CurrentAimTarget,
            CanInteractWithCurrentTarget(),
            CanCurrentlySizeShift(),
            false
        );
    }
    RefreshUX();
}

void ASizeShiftGun::LockSizeShift()
{
    // Disables size shifting and refreshes the UI state
    _CanSizeShift = false;

    RefreshUX();
}

bool ASizeShiftGun::CanSizeShift() const
{
    // Returns whether the size-shift ability has been unlocked
    return _CanSizeShift;
}

bool ASizeShiftGun::HasAimTarget() const
{
    // Checks whether the current aim trace has selected a target
    return _CurrentAimTarget != nullptr;
}

bool ASizeShiftGun::CanInteractWithCurrentTarget() const
{
    // Validates target type, range, character movement, and standing-box restrictions
    if (!HasAimTarget())
    {
        return false;
    }

    if (!IsWithinInteractionRange())
    {
        return false;
    }

    if (_CurrentAimTarget->GetInteractionType() ==
        E_BoxInteractionType::Immovable)
    {
        return false;
    }

    ASizeShiftCharacter* character =
        Cast<ASizeShiftCharacter>(GetOwner());

    if (character &&
        character->GetCharacterMovement()->IsFalling())
    {
        return false;
    }

    if (character &&
        character->GetStandingBox() == _CurrentAimTarget)
    {
        return false;
    }

    return true;
}

bool ASizeShiftGun::CanCurrentlySizeShift() const
{
    // Requires the ability, an aim target, and size-shift range
    return CanSizeShift() &&
        HasAimTarget() &&
        IsWithinSizeShiftRange();
}

void ASizeShiftGun::Interact()
{
    // Toggles the current interaction: stops an active one or validates and starts a new one
    if (_GunState == E_SizeShiftGunState::Interacting)
    {
        StopInteraction();
        return;
    }

    if (!CanInteractWithCurrentTarget())
     {
        if (_CurrentAimTarget &&
            IsWithinInteractionRange())
        {
            if (_UXComponent)
            {
                _UXComponent->PlayInvalidActionSound();
            }
        }

        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Interact Debug] AimTarget: %s"),
            IsValid(_CurrentAimTarget)
            ? *_CurrentAimTarget->GetName()
            : TEXT("None")
        );

        return;
    }

    _ActiveInteractionTarget =
        _CurrentAimTarget;

    StartInteraction();
}

void ASizeShiftGun::CancelInteraction()
{
    // Safely stops an interaction when another system requests cancellation
    if (_GunState != E_SizeShiftGunState::Interacting)
    {
        return;
    }

    StopInteraction();
}

bool ASizeShiftGun::IsWithinInteractionRange() const
{
    // Measures owner-to-target distance against the interaction limit
    if (!_CurrentAimTarget || !GetOwner())
    {
        return false;
    }

    const float distance = FVector::Dist(
        GetOwner()->GetActorLocation(),
        _CurrentAimTarget->GetActorLocation()
    );

    return distance <= _InteractionRange;
}

bool ASizeShiftGun::IsWithinSizeShiftRange() const
{
    // Measures owner-to-target distance against the size-shift limit
    if (!_CurrentAimTarget || !GetOwner())
    {
        return false;
    }

    const float distance = FVector::Dist(
        GetOwner()->GetActorLocation(),
        _CurrentAimTarget->GetActorLocation()
    );

    return distance <= _SizeShiftRange;
}

void ASizeShiftGun::StartInteraction()
{
    // Dispatches the active target to pickup/throw or push/pull setup, then updates gun state
    if (!_ActiveInteractionTarget)
    {
        SetGunState(E_SizeShiftGunState::Idle);
        return;
    }

    bool interactionStarted = false;

    switch (_ActiveInteractionTarget->GetInteractionType())
    {
    case E_BoxInteractionType::PickupThrow:

        PrintGunStatus(TEXT("INTERACT -> PICKUP / THROW"));

        interactionStarted = StartPickupHold();

        break;

    case E_BoxInteractionType::PushPull:

        PrintGunStatus(TEXT("INTERACT -> PUSH / PULL"));

        interactionStarted = StartPushPull();

        break;

    case E_BoxInteractionType::Immovable:

        PrintGunStatus(TEXT("INTERACT BLOCKED -> IMMOVABLE"));

        break;

    default:

        break;
    }

    if (interactionStarted)
    {
        SetGunState(
            E_SizeShiftGunState::Interacting
        );

        if (_UXComponent)
        {
            _UXComponent->PlayInteractSound();
        }
    }
    else
    {
        _ActiveInteractionTarget = nullptr;

        SetGunState(E_SizeShiftGunState::Idle);
    }
}

void ASizeShiftGun::StopInteraction()
{
    // Stops the matching interaction mode and clears the shared interaction state
    SetThrowAimMode(false);

    if (!_ActiveInteractionTarget)
    {
        ClearInteraction();
        return;
    }

    const E_BoxInteractionType interactionType =
        _ActiveInteractionTarget->GetInteractionType();

    switch (interactionType)
    {
    case E_BoxInteractionType::PickupThrow:

        StopPickupHold();

        break;

    case E_BoxInteractionType::PushPull:

        StopPushPull();

        break;

    default:

        break;
    }

    ClearInteraction();
}

void ASizeShiftGun::SetGunState(
    E_SizeShiftGunState newState)
{
    // Stores the current high-level gun state
    _GunState = newState;
}

void ASizeShiftGun::ClearInteraction()
{
    // Clears the active target and returns the gun to idle
    _ActiveInteractionTarget = nullptr;

    SetGunState(
        E_SizeShiftGunState::Idle
    );
}

FString ASizeShiftGun::GetGunStateName() const
{
    // Converts the current enum state into readable debug text
    switch (_GunState)
    {
    case E_SizeShiftGunState::Idle:
        return TEXT("IDLE");

    case E_SizeShiftGunState::Interacting:
        return TEXT("INTERACTING");

    default:
        return TEXT("UNKNOWN");
    }
}

APlayerController* ASizeShiftGun::GetPlayerController() const
{
    // Gets the controller associated with this gun's owning character
    if (!GetOwner())
    {
        return nullptr;
    }

    return Cast<APlayerController>(
        GetOwner()->GetInstigatorController()
    );
}

FVector ASizeShiftGun::GetHoldLocation() const
{
    // Calculates the desired held-box location in front of the player's camera
    APlayerController* playerController =
        GetPlayerController();

    if (!playerController)
    {
        return FVector::ZeroVector;
    }

    FVector cameraLocation;
    FRotator cameraRotation;

    playerController->GetPlayerViewPoint(
        cameraLocation,
        cameraRotation
    );

    return cameraLocation +
        cameraRotation.Vector() * _HoldDistance;
}

void ASizeShiftGun::UpdatePickupThrow()
{
    // Auto-drops distant held boxes; otherwise updates the held box's movement
    if (!IsHoldingPickupThrow() ||
        !_ActiveInteractionTarget ||
        !GetOwner())
    {
        return;
    }

    // ========================================================
    // AUTO DROP
    // ========================================================

    const float distanceFromPlayer =
        FVector::Dist(
            GetOwner()->GetActorLocation(),
            _ActiveInteractionTarget->GetActorLocation()
        );

    if (distanceFromPlayer > _AutoDropDistance)
    {
        Drop();
        return;
    }

    // ========================================================
    // HELD BOX MOVEMENT
    // ========================================================

    UpdateHeldBox();
}

void ASizeShiftGun::UpdateHeldBox()
{
    // Sweeps a held box toward its target, then slides it along blocking surfaces
    if (!_ActiveInteractionTarget)
    {
        return;
    }

    UStaticMeshComponent* boxMesh =
        _ActiveInteractionTarget->GetBoxMesh();

    if (!boxMesh)
    {
        return;
    }

    const FVector currentLocation =
        boxMesh->GetComponentLocation();

    const FVector targetLocation =
        GetHoldLocation();

    FVector movementDelta =
        targetLocation - currentLocation;

    if (movementDelta.IsNearlyZero())
    {
        return;
    }

    FHitResult hit;

    // ========================================================
    // FIRST SWEEP
    // ========================================================

    boxMesh->MoveComponent(
        movementDelta,
        _HeldBoxRotation.Quaternion(),
        true,
        &hit
    );

    // ========================================================
    // SLIDE ALONG SURFACE
    // ========================================================

    if (hit.bBlockingHit)
    {
        const float remainingTime =
            1.0f - hit.Time;

        if (remainingTime > 0.0f)
        {
            const FVector remainingMovement =
                movementDelta * remainingTime;

            const FVector slideMovement =
                FVector::VectorPlaneProject(
                    remainingMovement,
                    hit.Normal
                );

            if (!slideMovement.IsNearlyZero())
            {
                FHitResult slideHit;

                boxMesh->MoveComponent(
                    slideMovement,
                    _HeldBoxRotation.Quaternion(),
                    true,
                    &slideHit
                );
            }
        }
    }
}

bool ASizeShiftGun::StartPickupHold()
{
    // Validates a physics-enabled box, saves its rotation, and switches it to held physics
    if (!_ActiveInteractionTarget)
    {
        return false;
    }

    UStaticMeshComponent* boxMesh =
        _ActiveInteractionTarget->GetBoxMesh();

    if (!boxMesh)
    {
        return false;
    }

    if (!boxMesh->IsSimulatingPhysics())
    {
        PrintGunStatus(
            TEXT("PHYSICS HOLD FAILED -> Physics Disabled")
        );

        return false;
    }

    APlayerController* playerController =
        GetPlayerController();

    if (!playerController)
    {
        return false;
    }

    FVector cameraLocation;
    FRotator cameraRotation;

    playerController->GetPlayerViewPoint(
        cameraLocation,
        cameraRotation
    );

    _HoldDistance = _MinHoldDistance;

    _HeldBoxRotation =
        boxMesh->GetComponentRotation();

    SetHeldBoxPhysics(true);

    PrintGunStatus(
        TEXT("PHYSICS HOLD -> Sweep Movement")
    );

    return true;
}

void ASizeShiftGun::StopPickupHold()
{
    // Restores released-box physics and clears the saved held rotation
    if (!_ActiveInteractionTarget)
    {
        return;
    }

    UStaticMeshComponent* boxMesh =
        _ActiveInteractionTarget->GetBoxMesh();

    if (!boxMesh)
    {
        return;
    }

    SetHeldBoxPhysics(false);

    boxMesh->WakeAllRigidBodies();

    _HeldBoxRotation = FRotator::ZeroRotator;

    PrintGunStatus(
        TEXT("PHYSICS RELEASE")
    );
}

void ASizeShiftGun::SetHeldBoxPhysics(bool isHeld)
{
    // Uses query-only collision while held, then restores collision and simulation on release
    if (!_ActiveInteractionTarget)
    {
        return;
    }

    UStaticMeshComponent* boxMesh =
        _ActiveInteractionTarget->GetBoxMesh();

    if (!boxMesh)
    {
        return;
    }

    if (isHeld)
    {
        boxMesh->SetSimulatePhysics(false);

        boxMesh->SetCollisionEnabled(
            ECollisionEnabled::QueryOnly
        );
    }
    else
    {
        boxMesh->SetCollisionEnabled(
            ECollisionEnabled::QueryAndPhysics
        );

        boxMesh->SetSimulatePhysics(true);
    }
}

void ASizeShiftGun::Drop()
{
    // Releases a box only if pickup/throw mode is active
    if (!IsHoldingPickupThrow())
    {
        return;
    }

    StopInteraction();
}

bool ASizeShiftGun::IsHoldingPickupThrow() const
{
    // Returns whether the active interaction is a pickup/throw operation
    if (_GunState != E_SizeShiftGunState::Interacting)
    {
        return false;
    }

    if (!_ActiveInteractionTarget)
    {
        return false;
    }

    return _ActiveInteractionTarget->GetInteractionType()
        == E_BoxInteractionType::PickupThrow;
}

void ASizeShiftGun::Throw()
{
    // Restores physics, applies camera-forward velocity, plays feedback, and clears interaction
    SetThrowAimMode(false);

    if (_GunState != E_SizeShiftGunState::Interacting ||
        !_ActiveInteractionTarget)
    {
        return;
    }

    UStaticMeshComponent* boxMesh =
        _ActiveInteractionTarget->GetBoxMesh();

    if (!boxMesh)
    {
        return;
    }

    APlayerController* playerController =
        GetPlayerController();

    if (!playerController)
    {
        return;
    }

    FVector cameraLocation;
    FRotator cameraRotation;

    playerController->GetPlayerViewPoint(
        cameraLocation,
        cameraRotation
    );

    const FVector throwDirection =
        cameraRotation.Vector();

    const FVector throwVelocity =
        throwDirection * _ThrowSpeed;

    // ========================================================
    // RESTORE PHYSICS
    // ========================================================

    SetHeldBoxPhysics(false);

    // ========================================================
    // APPLY THROW
    // ========================================================

    boxMesh->SetPhysicsLinearVelocity(
        throwVelocity,
        false
    );

    boxMesh->WakeAllRigidBodies();

    PrintGunStatus(
        FString::Printf(
            TEXT(
                "THROW -> %s | Speed: %.1f"
            ),
            *_ActiveInteractionTarget->GetName(),
            _ThrowSpeed
        )
    );

    if (_UXComponent)
    {
        _UXComponent->PlayThrowSound();
    }

    ClearInteraction();
}

bool ASizeShiftGun::StartPushPull()
{
    // Validates the target, captures the initial horizontal facing direction, and begins control
    if (!_ActiveInteractionTarget || !GetOwner())
    {
        PrintGunStatus(
            TEXT("PUSH/PULL START FAILED -> Missing Target or Owner")
        );

        return false;
    }

    if (_ActiveInteractionTarget->GetInteractionType()
        != E_BoxInteractionType::PushPull)
    {
        PrintGunStatus(
            TEXT("PUSH/PULL START FAILED -> Wrong Interaction Type")
        );

        return false;
    }

    // ========================================================
    // INITIAL PLAYER FORWARD
    // ========================================================

    FVector initialDirection =
        GetOwner()->GetActorForwardVector();

    initialDirection.Z = 0.0f;

    if (initialDirection.IsNearlyZero())
    {
        PrintGunStatus(
            TEXT("PUSH/PULL START FAILED -> Invalid Player Direction")
        );

        return false;
    }

    initialDirection.Normalize();

    _CurrentPushPullDistance = _PushPullDistance;

    // ========================================================
    // INITIALIZE SMOOTH DIRECTION
    // ========================================================

    _SmoothedPushPullDirection =
        initialDirection;

    _HasPushPullDirection = true;

    // ========================================================
    // START INTERACTION
    // ========================================================

    PrintGunStatus(
        FString::Printf(
            TEXT(
                "PUSH/PULL START -> %s | Distance: %.1f | Direction: %s"
            ),
            *_ActiveInteractionTarget->GetName(),
            _PushPullDistance,
            *_SmoothedPushPullDirection.ToString()
        )
    );
    return true;
}

void ASizeShiftGun::UpdatePushPull(float deltaTime)
{
    // Smoothly moves a grounded physics box toward an anchor in front of the player
    if (!IsHoldingPushPull() ||
        !GetOwner())
    {
        return;
    }

    UPrimitiveComponent* boxComponent =
        _ActiveInteractionTarget->GetBoxMesh();

    if (!boxComponent ||
        !boxComponent->IsSimulatingPhysics())
    {
        return;
    }

    // ========================================================
    // CHECK GROUND SUPPORT
    // ========================================================

    if (!_ActiveInteractionTarget->HasPushPullGroundSupport())
    {
        PrintGunStatus(
            TEXT("PUSH/PULL AUTO RELEASE -> Box lost ground support")
        );

        StopInteraction();
        return;
    }

    if (!_HasPushPullDirection || deltaTime <= 0.0f)
    {
        return;
    }

    FVector desiredDirection =
        GetOwner()->GetActorForwardVector();

    desiredDirection.Z = 0.0f;

    if (desiredDirection.IsNearlyZero())
    {
        return;
    }

    desiredDirection.Normalize();

    _SmoothedPushPullDirection =
        FMath::VInterpTo(
            _SmoothedPushPullDirection,
            desiredDirection,
            deltaTime,
            _PushPullDirectionInterpSpeed
        );

    _SmoothedPushPullDirection.Normalize();

    // ========================================================
    // GET TARGET ANCHOR
    // ========================================================

    FVector anchorLocation =
        GetOwner()->GetActorLocation()
        + _SmoothedPushPullDirection * _CurrentPushPullDistance;

    // Keep box on ground plane
    anchorLocation.Z =
        _ActiveInteractionTarget->GetActorLocation().Z;

    // ========================================================
    // MOVE BOX TOWARD ANCHOR
    // ========================================================

    const FVector currentLocation =
        _ActiveInteractionTarget->GetActorLocation();

    FVector toAnchor =
        anchorLocation - currentLocation;

    const float distanceToAnchor =
        toAnchor.Size();

    if (distanceToAnchor <= 2.0f)
    {
        boxComponent->SetPhysicsLinearVelocity(
            FVector::ZeroVector,
            false
        );

        return;
    }

    // ========================================================
    // LIMIT FOLLOW SPEED
    // ========================================================

    toAnchor.Normalize();

    const FVector currentVelocity =
        boxComponent->GetPhysicsLinearVelocity();


    // ========================================================
    // DEAD ZONE
    // ========================================================

    if (distanceToAnchor <= _PushPullDeadZone)
    {
        const FVector newVelocity =
            FMath::VInterpTo(
                currentVelocity,
                FVector::ZeroVector,
                deltaTime,
                _PushPullVelocityInterpSpeed
            );

        boxComponent->SetPhysicsLinearVelocity(
            newVelocity,
            false
        );

        return;
    }


    // ========================================================
    // TARGET VELOCITY
    // ========================================================

    const FVector targetVelocity =
        toAnchor * _PushPullFollowSpeed;


    // ========================================================
    // SMOOTH VELOCITY
    // ========================================================

    const FVector newVelocity =
        FMath::VInterpTo(
            currentVelocity,
            targetVelocity,
            deltaTime,
            _PushPullVelocityInterpSpeed
        );

    boxComponent->SetPhysicsLinearVelocity(
        newVelocity,
        false
    );

    UpdatePushPullFacing();
}

void ASizeShiftGun::StopPushPull()
{
    // Preserves vertical velocity, clears horizontal movement, and resets push/pull state
    SetThrowAimMode(false);

    _CurrentPushPullDistance = 0.0f;

    if (_ActiveInteractionTarget)
    {
        UPrimitiveComponent* boxComponent =
            _ActiveInteractionTarget->GetBoxMesh();

        if (boxComponent)
        {
            const FVector currentVelocity =
                boxComponent->GetPhysicsLinearVelocity();


            boxComponent->SetPhysicsLinearVelocity(
                FVector(
                    0.0f,
                    0.0f,
                    currentVelocity.Z
                ),
                false
            );

            boxComponent->WakeAllRigidBodies();
        }

        PrintGunStatus(
            FString::Printf(
                TEXT(
                    "PUSH/PULL STOP -> %s"
                ),
                *_ActiveInteractionTarget->GetName()
            )
        );
    }

    _SmoothedPushPullDirection =
        FVector::ZeroVector;

    _HasPushPullDirection = false;
}

void ASizeShiftGun::UpdatePushPullFacing()
{
    // Rotates the active box toward the player and removes angular physics velocity
    if (!_ActiveInteractionTarget || !GetOwner())
    {
        return;
    }

    UStaticMeshComponent* boxMesh =
        _ActiveInteractionTarget->GetBoxMesh();

    if (!boxMesh)
    {
        return;
    }

    FVector toPlayer =
        GetOwner()->GetActorLocation() -
        _ActiveInteractionTarget->GetActorLocation();

    toPlayer.Z = 0.0f;

    if (toPlayer.IsNearlyZero())
    {
        return;
    }

    toPlayer.Normalize();

    FRotator targetRotation =
        toPlayer.Rotation() +
        _PushPullFacingRotationOffset;

    boxMesh->SetWorldRotation(
        targetRotation,
        false,
        nullptr,
        ETeleportType::TeleportPhysics
    );

    boxMesh->SetPhysicsAngularVelocityInDegrees(
        FVector::ZeroVector
    );
}

bool ASizeShiftGun::IsHoldingPushPull() const
{
    // Returns whether the active interaction is a push/pull operation
    return _GunState == E_SizeShiftGunState::Interacting &&
        _ActiveInteractionTarget &&
        _ActiveInteractionTarget->GetInteractionType() ==
        E_BoxInteractionType::PushPull;
}

void ASizeShiftGun::PushPullBurst()
{
    // Ends push/pull control and launches the box forward with an impulse
    if (!IsHoldingPushPull() || !GetOwner())
    {
        return;
    }

    UStaticMeshComponent* boxMesh =
        _ActiveInteractionTarget->GetBoxMesh();

    if (!boxMesh || !boxMesh->IsSimulatingPhysics())
    {
        return;
    }

    FVector pushDirection =
        GetOwner()->GetActorForwardVector();

    pushDirection.Z = 0.0f;

    if (pushDirection.IsNearlyZero())
    {
        return;
    }

    pushDirection.Normalize();

    StopPushPull();

    ClearInteraction();

    boxMesh->WakeAllRigidBodies();

    boxMesh->AddImpulse(
        pushDirection * _PushPullBurstImpulse
    );

    if (_UXComponent)
    {
        _UXComponent->PlayThrowSound();
    }
}

void ASizeShiftGun::AdjustHoldDistance(float scrollValue)
{
    // Applies scroll input to the held-box distance within configured limits
    if (!IsHoldingPickupThrow() ||
        FMath::IsNearlyZero(scrollValue))
    {
        return;
    }

    _HoldDistance = FMath::Clamp(
        _HoldDistance +
        scrollValue * _HoldDistanceStep,
        _MinHoldDistance,
        _MaxHoldDistance
    );
}

void ASizeShiftGun::ToggleThrowAimMode()
{
    // Toggles throw-aim presentation only while holding a pickup/throw box
    if (!IsHoldingPickupThrow())
    {
        return;
    }

    SetThrowAimMode(!_IsThrowAimMode);
}

void ASizeShiftGun::TryThrow()
{
    // Throws only after throw-aim mode has been enabled
    if (!IsHoldingPickupThrow() ||
        !_IsThrowAimMode)
    {
        return;
    }

    SetThrowAimMode(false);

    Throw();
}

void ASizeShiftGun::SetThrowAimMode(
    bool newThrowAimMode)
{
    // Saves/restores hold distance and shows/hides the preview for the requested aim mode
    if (_IsThrowAimMode == newThrowAimMode)
    {
        return;
    }

    if (newThrowAimMode)
    {
        _ThrowAimPreviousHoldDistance =
            _HoldDistance;

        _IsThrowAimMode = true;

        _HoldDistance =
            _DefaultHoldDistance - 100.0f;

        ShowHeldBoxPreview();
    }
    else
    {
        _IsThrowAimMode = false;

        _HoldDistance =
            _ThrowAimPreviousHoldDistance;

        HideHeldBoxPreview();
    }
}

void ASizeShiftGun::ShowHeldBoxPreview()
{
    // Copies the held box mesh and materials into a visible, non-physical preview
    if (!_ActiveInteractionTarget ||
        !_HeldBoxPreview)
    {
        return;
    }

    UStaticMeshComponent* realBoxMesh =
        _ActiveInteractionTarget->GetBoxMesh();

    if (!realBoxMesh)
    {
        return;
    }

    _HeldBoxPreview->SetStaticMesh(
        realBoxMesh->GetStaticMesh()
    );

    for (int32 index = 0;
        index < realBoxMesh->GetNumMaterials();
        ++index)
    {
        _HeldBoxPreview->SetMaterial(
            index,
            realBoxMesh->GetMaterial(index)
        );
    }

    _HeldBoxPreview->SetRelativeLocation(
        _ThrowPreviewOffset
    );

    realBoxMesh->SetVisibility(false);

    _HeldBoxPreview->SetVisibility(true);
}

void ASizeShiftGun::HideHeldBoxPreview()
{
    // Hides the preview and restores visibility of the real held box
    if (_HeldBoxPreview)
    {
        _HeldBoxPreview->SetVisibility(false);
    }

    if (_ActiveInteractionTarget)
    {
        UStaticMeshComponent* realBoxMesh =
            _ActiveInteractionTarget->GetBoxMesh();

        if (realBoxMesh)
        {
            realBoxMesh->SetVisibility(true);
        }
    }
}

void ASizeShiftGun::RefreshUX()
{
    // Updates the target outline, prompts, and on-screen prompt position from current gun state
    if (!_UXComponent)
    {
        return;
    }

    const bool canInteract =
        CanInteractWithCurrentTarget();

    const bool canCurrentlySizeShift =
        CanCurrentlySizeShift();

    const bool isHolding =
        IsHoldingPickupThrow() ||
        IsHoldingPushPull();

    // ============================================================
    // Outline
    // ============================================================

    _UXComponent->UpdateTargetOutline(
        _CurrentAimTarget,
        canInteract,
        canCurrentlySizeShift,
        isHolding
    );

    // ============================================================
    // Prompt
    // ============================================================

    bool showInteract = false;
    bool showSizeUp = false;
    bool showSizeDown = false;

    if (_GunState == E_SizeShiftGunState::Idle &&
        _CurrentAimTarget &&
        IsWithinInteractionRange())
    {
        switch (_CurrentAimTarget->GetBoxSize())
        {
        case E_BoxSizeType::Small:

            showInteract = true;
            showSizeUp = canCurrentlySizeShift;

            break;

        case E_BoxSizeType::Medium:

            showInteract = true;
            showSizeUp = canCurrentlySizeShift;
            showSizeDown = canCurrentlySizeShift;

            break;

        case E_BoxSizeType::Large:

            showSizeDown = canCurrentlySizeShift;

            break;

        default:

            break;
        }
    }

    _UXComponent->SetPromptState(
        showInteract,
        showSizeUp,
        showSizeDown
    );

    // ============================================================
    // Prompt Position
    // ============================================================

    if (_CurrentAimTarget)
    {
        APlayerController* playerController =
            GetWorld()->GetFirstPlayerController();

        if (playerController)
        {
            FVector worldLocation =
                _CurrentAimTarget->GetActorLocation();

            worldLocation.Z += _PromptHeightOffset;

            FVector2D screenPosition;

            if (playerController->ProjectWorldLocationToScreen(
                worldLocation,
                screenPosition))
            {
                _UXComponent->SetPromptPosition(
                    screenPosition
                );
            }
        }
    }
}

void ASizeShiftGun::PrintGunStatus(
    const FString& message) const
{
    // Logs and optionally displays the current state and active target for troubleshooting
    if (!_EnableDebug)
    {
        return;
    }

    const FString targetName =
        _ActiveInteractionTarget
        ? _ActiveInteractionTarget->GetName()
        : TEXT("None");

    const FString debugMessage =
        FString::Printf(
            TEXT("[SizeShiftGun] %s | State: %s | Target: %s"),
            *message,
            *GetGunStateName(),
            *targetName
        );

    UE_LOG(
        LogTemp,
        Log,
        TEXT("%s"),
        *debugMessage
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.0f,
            FColor::White,
            debugMessage
        );
    }
}