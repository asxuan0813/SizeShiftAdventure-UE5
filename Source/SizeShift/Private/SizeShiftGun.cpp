// Fill out your copyright notice in the Description page of Project Settings.


#include "SizeShiftGun.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "SizeShiftBox.h"

// Sets default values
ASizeShiftGun::ASizeShiftGun()
{
    PrimaryActorTick.bCanEverTick = true;

    GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("GunMesh")
    );

    RootComponent = GunMesh;

    HeldBoxPreview =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("HeldBoxPreview")
        );

    HeldBoxPreview->SetupAttachment(GunMesh);

    HeldBoxPreview->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    HeldBoxPreview->SetSimulatePhysics(false);

    HeldBoxPreview->SetCastShadow(false);

    HeldBoxPreview->SetRenderCustomDepth(true);

    HeldBoxPreview->SetCustomDepthStencilValue(1);

    HeldBoxPreview->SetVisibility(false);
    
    CurrentAimTarget = nullptr;
    ActiveInteractionTarget = nullptr;

    GunState = ESizeShiftGunState::Idle;

}

// Called when the game starts or when spawned
void ASizeShiftGun::BeginPlay()
{
	Super::BeginPlay();

    SetGunState(
        ESizeShiftGunState::Idle
    );

    DefaultHoldDistance = HoldDistance;
	
}

// Called every frame
void ASizeShiftGun::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAimTarget();

    UpdatePickupThrow();

    UpdatePushPull(DeltaTime);
}

// ============================================================
// Aim
// ============================================================

ASizeShiftBox* ASizeShiftGun::GetAimTarget() const
{
    APlayerController* PlayerController =
        GetPlayerController();

    if (!PlayerController)
    {
        return nullptr;
    }

    FVector CameraLocation;
    FRotator CameraRotation;

    PlayerController->GetPlayerViewPoint(
        CameraLocation,
        CameraRotation
    );

    const FVector Start = CameraLocation;

    const FVector End =
        Start +
        CameraRotation.Vector() * TargetTraceDistance;

    FHitResult Hit;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.AddIgnoredActor(GetOwner());

    const bool bHit =
        GetWorld()->LineTraceSingleByChannel(
            Hit,
            Start,
            End,
            ECC_Visibility,
            Params
        );

    if (bEnableDebug)
    {
        DrawDebugLine(
            GetWorld(),
            Start,
            End,
            bHit ? FColor::Green : FColor::Red,
            false,
            0.0f,
            0,
            0.5f
        );
    }

    if (!bHit)
    {
        return nullptr;
    }

    return Cast<ASizeShiftBox>(Hit.GetActor());
}

void ASizeShiftGun::UpdateAimTarget()
{
    ASizeShiftBox* NewAimTarget = GetAimTarget();

    if (NewAimTarget != CurrentAimTarget)
    {
        CurrentAimTarget = NewAimTarget;

        if (CurrentAimTarget)
        {
            PrintGunStatus(
                FString::Printf(
                    TEXT("AIM TARGET -> %s"),
                    *CurrentAimTarget->GetName()
                )
            );
        }
        else
        {
            PrintGunStatus(
                TEXT("AIM TARGET -> None")
            );
        }
    }
}

// ============================================================
// Size Control
// ============================================================

void ASizeShiftGun::IncreaseSize()
{
    if (GunState == ESizeShiftGunState::Interacting)
    {
        PrintGunStatus(
            TEXT("INCREASE SIZE BLOCKED -> Interacting")
        );

        return;
    }

    if (!CurrentAimTarget)
    {
        PrintGunStatus(
            TEXT("INCREASE SIZE FAILED -> No Box")
        );

        return;
    }

    CurrentAimTarget->IncreaseBoxSize();

    PrintGunStatus(
        FString::Printf(
            TEXT("SIZE UP -> %s"),
            *CurrentAimTarget->GetName()
        )
    );
}

void ASizeShiftGun::DecreaseSize()
{
    if (GunState == ESizeShiftGunState::Interacting)
    {
        PrintGunStatus(
            TEXT("DECREASE SIZE BLOCKED -> Interacting")
        );

        return;
    }

    if (!CurrentAimTarget)
    {
        PrintGunStatus(
            TEXT("DECREASE SIZE FAILED -> No Box")
        );

        return;
    }

    CurrentAimTarget->DecreaseBoxSize();

    PrintGunStatus(
        FString::Printf(
            TEXT("SIZE DOWN -> %s"),
            *CurrentAimTarget->GetName()
        )
    );
}

// ============================================================
// Interaction Flow
// ============================================================

void ASizeShiftGun::Interact()
{
    // ========================================
    // CURRENTLY INTERACTING
    // ========================================

    if (GunState == ESizeShiftGunState::Interacting)
    {
        StopInteraction();

        return;
    }

    // ========================================
    // NO TARGET
    // ========================================

    if (!CurrentAimTarget)
    {
        PrintGunStatus(
            TEXT("INTERACT FAILED -> No Box")
        );

        return;
    }

    // ========================================
    // RANGE CHECK
    // ========================================

    if (!IsWithinInteractionRange())
    {
        PrintGunStatus(
            TEXT("INTERACT FAILED -> Box is too far")
        );

        return;
    }

    // ========================================
    // START
    // ========================================

    ActiveInteractionTarget =
        CurrentAimTarget;

    StartInteraction();
}

void ASizeShiftGun::CancelInteraction()
{
    if (GunState != ESizeShiftGunState::Interacting)
    {
        return;
    }

    StopInteraction();
}

bool ASizeShiftGun::IsWithinInteractionRange() const
{
    if (!CurrentAimTarget || !GetOwner())
    {
        return false;
    }

    const float Distance = FVector::Dist(
        GetOwner()->GetActorLocation(),
        CurrentAimTarget->GetActorLocation()
    );

    return Distance <= InteractionRange;
}

void ASizeShiftGun::StartInteraction()
{
    if (!ActiveInteractionTarget)
    {
        SetGunState(ESizeShiftGunState::Idle);
        return;
    }

    bool bInteractionStarted = false;

    switch (ActiveInteractionTarget->GetInteractionType())
    {
    case EBoxInteractionType::PickupThrow:

        PrintGunStatus(TEXT("INTERACT -> PICKUP / THROW"));

        bInteractionStarted = StartPickupHold();

        break;

    case EBoxInteractionType::PushPull:

        PrintGunStatus(TEXT("INTERACT -> PUSH / PULL"));

        bInteractionStarted = StartPushPull();

        break;

    case EBoxInteractionType::Immovable:

        PrintGunStatus(TEXT("INTERACT BLOCKED -> IMMOVABLE"));

        break;

    default:

        break;
    }

    if (bInteractionStarted)
    {
        SetGunState(ESizeShiftGunState::Interacting);
    }
    else
    {
        ActiveInteractionTarget = nullptr;

        SetGunState(ESizeShiftGunState::Idle);
    }
}

void ASizeShiftGun::StopInteraction()
{
    SetThrowAimMode(false);

    if (!ActiveInteractionTarget)
    {
        ClearInteraction();
        return;
    }

    const EBoxInteractionType InteractionType =
        ActiveInteractionTarget->GetInteractionType();

    switch (InteractionType)
    {
    case EBoxInteractionType::PickupThrow:

        StopPickupHold();

        break;

    case EBoxInteractionType::PushPull:

        StopPushPull();

        break;

    default:

        break;
    }

    ClearInteraction();
}

void ASizeShiftGun::SetGunState(
    ESizeShiftGunState NewState)
{
    GunState = NewState;
}

void ASizeShiftGun::ClearInteraction()
{
    ActiveInteractionTarget = nullptr;

    SetGunState(
        ESizeShiftGunState::Idle
    );
}

FString ASizeShiftGun::GetGunStateName() const
{
    switch (GunState)
    {
    case ESizeShiftGunState::Idle:
        return TEXT("IDLE");

    case ESizeShiftGunState::Interacting:
        return TEXT("INTERACTING");

    default:
        return TEXT("UNKNOWN");
    }
}

APlayerController* ASizeShiftGun::GetPlayerController() const
{
    if (!GetOwner())
    {
        return nullptr;
    }

    return Cast<APlayerController>(
        GetOwner()->GetInstigatorController()
    );
}

// ============================================================
// Pickup / Throw
// ============================================================

FVector ASizeShiftGun::GetHoldLocation() const
{
    APlayerController* PlayerController =
        GetPlayerController();

    if (!PlayerController)
    {
        return FVector::ZeroVector;
    }

    FVector CameraLocation;
    FRotator CameraRotation;

    PlayerController->GetPlayerViewPoint(
        CameraLocation,
        CameraRotation
    );

    return CameraLocation +
        CameraRotation.Vector() * HoldDistance;
}

void ASizeShiftGun::UpdatePickupThrow()
{
    if (!IsHoldingPickupThrow() ||
        !ActiveInteractionTarget ||
        !GetOwner())
    {
        return;
    }

    // ========================================================
    // AUTO DROP
    // ========================================================

    const float DistanceFromPlayer =
        FVector::Dist(
            GetOwner()->GetActorLocation(),
            ActiveInteractionTarget->GetActorLocation()
        );

    if (DistanceFromPlayer > AutoDropDistance)
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
    if (!ActiveInteractionTarget)
    {
        return;
    }

    UStaticMeshComponent* BoxMesh =
        ActiveInteractionTarget->GetBoxMesh();

    if (!BoxMesh)
    {
        return;
    }

    const FVector CurrentLocation =
        BoxMesh->GetComponentLocation();

    const FVector TargetLocation =
        GetHoldLocation();

    FVector MovementDelta =
        TargetLocation - CurrentLocation;

    if (MovementDelta.IsNearlyZero())
    {
        return;
    }

    FHitResult Hit;

    // ========================================================
    // FIRST SWEEP
    // ========================================================

    BoxMesh->MoveComponent(
        MovementDelta,
        HeldBoxRotation.Quaternion(),
        true,
        &Hit
    );

    // ========================================================
    // SLIDE ALONG SURFACE
    // ========================================================

    if (Hit.bBlockingHit)
    {
        const float RemainingTime =
            1.0f - Hit.Time;

        if (RemainingTime > 0.0f)
        {
            const FVector RemainingMovement =
                MovementDelta * RemainingTime;

            const FVector SlideMovement =
                FVector::VectorPlaneProject(
                    RemainingMovement,
                    Hit.Normal
                );

            if (!SlideMovement.IsNearlyZero())
            {
                FHitResult SlideHit;

                BoxMesh->MoveComponent(
                    SlideMovement,
                    HeldBoxRotation.Quaternion(),
                    true,
                    &SlideHit
                );
            }
        }
    }
}

bool ASizeShiftGun::StartPickupHold()
{
    if (!ActiveInteractionTarget)
    {
        return false;
    }

    UStaticMeshComponent* BoxMesh =
        ActiveInteractionTarget->GetBoxMesh();

    if (!BoxMesh)
    {
        return false;
    }

    if (!BoxMesh->IsSimulatingPhysics())
    {
        PrintGunStatus(
            TEXT("PHYSICS HOLD FAILED -> Physics Disabled")
        );

        return false;
    }

    APlayerController* PlayerController =
        GetPlayerController();

    if (!PlayerController)
    {
        return false;
    }


    FVector CameraLocation;
    FRotator CameraRotation;

    PlayerController->GetPlayerViewPoint(
        CameraLocation,
        CameraRotation
    );

    HoldDistance = FVector::Dist(
        CameraLocation,
        BoxMesh->GetComponentLocation()
    );

    HeldBoxRotation =
        BoxMesh->GetComponentRotation();

    SetHeldBoxPhysics(true);

    PrintGunStatus(
        TEXT("PHYSICS HOLD -> Sweep Movement")
    );

    return true;
}

void ASizeShiftGun::StopPickupHold()
{
    if (!ActiveInteractionTarget)
    {
        return;
    }

    UStaticMeshComponent* BoxMesh =
        ActiveInteractionTarget->GetBoxMesh();

    if (!BoxMesh)
    {
        return;
    }

    SetHeldBoxPhysics(false);

    BoxMesh->WakeAllRigidBodies();

    HeldBoxRotation = FRotator::ZeroRotator;

    PrintGunStatus(
        TEXT("PHYSICS RELEASE")
    );
}

void ASizeShiftGun::SetHeldBoxPhysics(bool bIsHeld)
{
    if (!ActiveInteractionTarget)
    {
        return;
    }

    UStaticMeshComponent* BoxMesh =
        ActiveInteractionTarget->GetBoxMesh();

    if (!BoxMesh)
    {
        return;
    }

    if (bIsHeld)
    {
        BoxMesh->SetSimulatePhysics(false);

        BoxMesh->SetCollisionEnabled(
            ECollisionEnabled::QueryOnly
        );
    }
    else
    {
        BoxMesh->SetCollisionEnabled(
            ECollisionEnabled::QueryAndPhysics
        );

        BoxMesh->SetSimulatePhysics(true);
    }
}

void ASizeShiftGun::Drop()
{
    if (!IsHoldingPickupThrow())
    {
        return;
    }

    StopInteraction();
}

bool ASizeShiftGun::IsHoldingPickupThrow() const
{
    if (GunState != ESizeShiftGunState::Interacting)
    {
        return false;
    }

    if (!ActiveInteractionTarget)
    {
        return false;
    }

    return ActiveInteractionTarget->GetInteractionType()
        == EBoxInteractionType::PickupThrow;
}

void ASizeShiftGun::Throw()
{
    SetThrowAimMode(false);

    if (GunState != ESizeShiftGunState::Interacting ||
        !ActiveInteractionTarget)
    {
        return;
    }

    UStaticMeshComponent* BoxMesh =
        ActiveInteractionTarget->GetBoxMesh();

    if (!BoxMesh)
    {
        return;
    }

    APlayerController* PlayerController =
        GetPlayerController();

    if (!PlayerController)
    {
        return;
    }

    FVector CameraLocation;
    FRotator CameraRotation;

    PlayerController->GetPlayerViewPoint(
        CameraLocation,
        CameraRotation
    );

    const FVector ThrowDirection =
        CameraRotation.Vector();

    const FVector ThrowVelocity =
        ThrowDirection * ThrowSpeed;

    // ========================================================
    // RESTORE PHYSICS
    // ========================================================

    SetHeldBoxPhysics(false);

    // ========================================================
    // APPLY THROW
    // ========================================================

    BoxMesh->SetPhysicsLinearVelocity(
        ThrowVelocity,
        false
    );

    BoxMesh->WakeAllRigidBodies();

    PrintGunStatus(
        FString::Printf(
            TEXT(
                "THROW -> %s | Speed: %.1f"
            ),
            *ActiveInteractionTarget->GetName(),
            ThrowSpeed
        )
    );

    ClearInteraction();
}

// ============================================================
// Push / Pull
// ============================================================

bool ASizeShiftGun::StartPushPull()
{
    if (!ActiveInteractionTarget || !GetOwner())
    {
        PrintGunStatus(
            TEXT("PUSH/PULL START FAILED -> Missing Target or Owner")
        );

        return false;
    }

    if (ActiveInteractionTarget->GetInteractionType()
        != EBoxInteractionType::PushPull)
    {
        PrintGunStatus(
            TEXT("PUSH/PULL START FAILED -> Wrong Interaction Type")
        );

        return false;
    }

    // ========================================================
    // INITIAL PLAYER FORWARD
    // ========================================================

    FVector InitialDirection =
        GetOwner()->GetActorForwardVector();

    InitialDirection.Z = 0.0f;

    if (InitialDirection.IsNearlyZero())
    {
        PrintGunStatus(
            TEXT("PUSH/PULL START FAILED -> Invalid Player Direction")
        );

        return false;
    }

    InitialDirection.Normalize();

    CurrentPushPullDistance = PushPullDistance;

    // ========================================================
    // INITIALIZE SMOOTH DIRECTION
    // ========================================================

    SmoothedPushPullDirection =
        InitialDirection;

    bHasPushPullDirection = true;

    // ========================================================
    // START INTERACTION
    // ========================================================

    PrintGunStatus(
        FString::Printf(
            TEXT(
                "PUSH/PULL START -> %s | Distance: %.1f | Direction: %s"
            ),
            *ActiveInteractionTarget->GetName(),
            PushPullDistance,
            *SmoothedPushPullDirection.ToString()
        )
    );
    return true;
}

void ASizeShiftGun::UpdatePushPull(float DeltaTime)
{
    if (!IsHoldingPushPull() ||
        !GetOwner())
    {
        return;
    }

    UPrimitiveComponent* BoxComponent =
        ActiveInteractionTarget->GetBoxMesh();

    if (!BoxComponent ||
        !BoxComponent->IsSimulatingPhysics())
    {
        return;
    }

    if (!bHasPushPullDirection || DeltaTime <= 0.0f)
    {
        return;
    }

    FVector DesiredDirection =
        GetOwner()->GetActorForwardVector();

    DesiredDirection.Z = 0.0f;

    if (DesiredDirection.IsNearlyZero())
    {
        return;
    }

    DesiredDirection.Normalize();

    SmoothedPushPullDirection =
        FMath::VInterpTo(
            SmoothedPushPullDirection,
            DesiredDirection,
            DeltaTime,
            PushPullDirectionInterpSpeed
        );

    SmoothedPushPullDirection.Normalize();

    // ========================================================
    // GET TARGET ANCHOR
    // ========================================================

    FVector AnchorLocation =
        GetOwner()->GetActorLocation()
        + SmoothedPushPullDirection * CurrentPushPullDistance;

    // Keep box on ground plane
    AnchorLocation.Z =
        ActiveInteractionTarget->GetActorLocation().Z;

    // ========================================================
    // MOVE BOX TOWARD ANCHOR
    // ========================================================

    const FVector CurrentLocation =
        ActiveInteractionTarget->GetActorLocation();

    FVector ToAnchor =
        AnchorLocation - CurrentLocation;

    const float DistanceToAnchor =
        ToAnchor.Size();

    if (DistanceToAnchor <= 2.0f)
    {
        BoxComponent->SetPhysicsLinearVelocity(
            FVector::ZeroVector,
            false
        );

        return;
    }

    // ========================================================
    // LIMIT FOLLOW SPEED
    // ========================================================

    ToAnchor.Normalize();

    const FVector CurrentVelocity =
        BoxComponent->GetPhysicsLinearVelocity();


    // ========================================================
    // DEAD ZONE
    // ========================================================

    if (DistanceToAnchor <= PushPullDeadZone)
    {
        const FVector NewVelocity =
            FMath::VInterpTo(
                CurrentVelocity,
                FVector::ZeroVector,
                DeltaTime,
                PushPullVelocityInterpSpeed
            );

        BoxComponent->SetPhysicsLinearVelocity(
            NewVelocity,
            false
        );

        return;
    }


    // ========================================================
    // TARGET VELOCITY
    // ========================================================

    const FVector TargetVelocity =
        ToAnchor * PushPullFollowSpeed;


    // ========================================================
    // SMOOTH VELOCITY
    // ========================================================

    const FVector NewVelocity =
        FMath::VInterpTo(
            CurrentVelocity,
            TargetVelocity,
            DeltaTime,
            PushPullVelocityInterpSpeed
        );

    BoxComponent->SetPhysicsLinearVelocity(
        NewVelocity,
        false
    );

    UpdatePushPullFacing();
}

void ASizeShiftGun::StopPushPull()
{
    SetThrowAimMode(false);

    CurrentPushPullDistance = 0.0f;

    if (ActiveInteractionTarget)
    {
        UPrimitiveComponent* BoxComponent =
            ActiveInteractionTarget->GetBoxMesh();

        if (BoxComponent)
        {
            BoxComponent->SetPhysicsLinearVelocity(
                FVector::ZeroVector,
                false
            );

            BoxComponent->WakeAllRigidBodies();
        }

        PrintGunStatus(
            FString::Printf(
                TEXT(
                    "PUSH/PULL STOP -> %s"
                ),
                *ActiveInteractionTarget->GetName()
            )
        );
    }

    SmoothedPushPullDirection =
        FVector::ZeroVector;

    bHasPushPullDirection = false;
}

void ASizeShiftGun::UpdatePushPullFacing()
{
    if (!ActiveInteractionTarget || !GetOwner())
    {
        return;
    }

    UStaticMeshComponent* BoxMesh =
        ActiveInteractionTarget->GetBoxMesh();

    if (!BoxMesh)
    {
        return;
    }

    FVector ToPlayer =
        GetOwner()->GetActorLocation() -
        ActiveInteractionTarget->GetActorLocation();

    ToPlayer.Z = 0.0f;

    if (ToPlayer.IsNearlyZero())
    {
        return;
    }

    ToPlayer.Normalize();

    FRotator TargetRotation =
        ToPlayer.Rotation() +
        PushPullFacingRotationOffset;

    BoxMesh->SetWorldRotation(
        TargetRotation,
        false,
        nullptr,
        ETeleportType::TeleportPhysics
    );

    BoxMesh->SetPhysicsAngularVelocityInDegrees(
        FVector::ZeroVector
    );
}

bool ASizeShiftGun::IsHoldingPushPull() const
{
    return GunState == ESizeShiftGunState::Interacting &&
        ActiveInteractionTarget &&
        ActiveInteractionTarget->GetInteractionType() ==
        EBoxInteractionType::PushPull;
}

void ASizeShiftGun::PushPullBurst()
{
    if (!IsHoldingPushPull() || !GetOwner())
    {
        return;
    }

    UStaticMeshComponent* BoxMesh =
        ActiveInteractionTarget->GetBoxMesh();

    if (!BoxMesh || !BoxMesh->IsSimulatingPhysics())
    {
        return;
    }

    FVector PushDirection =
        GetOwner()->GetActorForwardVector();

    PushDirection.Z = 0.0f;

    if (PushDirection.IsNearlyZero())
    {
        return;
    }

    PushDirection.Normalize();

    StopPushPull();

    ClearInteraction();

    BoxMesh->WakeAllRigidBodies();

    BoxMesh->AddImpulse(
        PushDirection * PushPullBurstImpulse
    );
}

// ============================================================
// Grab Distance
// ============================================================

void ASizeShiftGun::AdjustHoldDistance(float ScrollValue)
{
    if (!IsHoldingPickupThrow() ||
        FMath::IsNearlyZero(ScrollValue))
    {
        return;
    }

    HoldDistance = FMath::Clamp(
        HoldDistance +
        ScrollValue * HoldDistanceStep,
        MinHoldDistance,
        MaxHoldDistance
    );
}

//=============================================================

void ASizeShiftGun::ToggleThrowAimMode()
{
    if (!IsHoldingPickupThrow())
    {
        return;
    }

    SetThrowAimMode(!bIsThrowAimMode);
}

void ASizeShiftGun::TryThrow()
{
    if (!IsHoldingPickupThrow() ||
        !bIsThrowAimMode)
    {
        return;
    }

    SetThrowAimMode(false);

    Throw();
}

void ASizeShiftGun::SetThrowAimMode(
    bool bNewThrowAimMode)
{
    if (bIsThrowAimMode == bNewThrowAimMode)
    {
        return;
    }

    if (bNewThrowAimMode)
    {
        ThrowAimPreviousHoldDistance =
            HoldDistance;

        bIsThrowAimMode = true;

        HoldDistance =
            DefaultHoldDistance - 100.0f;

        ShowHeldBoxPreview();
    }
    else
    {
        bIsThrowAimMode = false;

        HoldDistance =
            ThrowAimPreviousHoldDistance;

        HideHeldBoxPreview();
    }
}

// =============================================================
// Show Held Box Preview
// =============================================================

void ASizeShiftGun::ShowHeldBoxPreview()
{
    if (!ActiveInteractionTarget ||
        !HeldBoxPreview)
    {
        return;
    }

    UStaticMeshComponent* RealBoxMesh =
        ActiveInteractionTarget->GetBoxMesh();

    if (!RealBoxMesh)
    {
        return;
    }

    HeldBoxPreview->SetStaticMesh(
        RealBoxMesh->GetStaticMesh()
    );

    for (int32 Index = 0;
        Index < RealBoxMesh->GetNumMaterials();
        ++Index)
    {
        HeldBoxPreview->SetMaterial(
            Index,
            RealBoxMesh->GetMaterial(Index)
        );
    }

    HeldBoxPreview->SetRelativeLocation(
        ThrowPreviewOffset
    );

    RealBoxMesh->SetVisibility(false);

    HeldBoxPreview->SetVisibility(true);
}

void ASizeShiftGun::HideHeldBoxPreview()
{
    if (HeldBoxPreview)
    {
        HeldBoxPreview->SetVisibility(false);
    }

    if (ActiveInteractionTarget)
    {
        UStaticMeshComponent* RealBoxMesh =
            ActiveInteractionTarget->GetBoxMesh();

        if (RealBoxMesh)
        {
            RealBoxMesh->SetVisibility(true);
        }
    }
}


// ============================================================
// Debug
// ============================================================

void ASizeShiftGun::PrintGunStatus(
    const FString& Message) const
{
    if (!bEnableDebug)
    {
        return;
    }

    const FString TargetName =
        ActiveInteractionTarget
        ? ActiveInteractionTarget->GetName()
        : TEXT("None");

    const FString DebugMessage =
        FString::Printf(
            TEXT("[SizeShiftGun] %s | State: %s | Target: %s"),
            *Message,
            *GetGunStateName(),
            *TargetName
        );

    UE_LOG(
        LogTemp,
        Log,
        TEXT("%s"),
        *DebugMessage
    );

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.0f,
            FColor::White,
            DebugMessage
        );
    }
}




