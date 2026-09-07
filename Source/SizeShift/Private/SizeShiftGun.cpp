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

    PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(
        TEXT("PhysicsHandle")
    );

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

    // ========================================================
    // PICKUP / THROW
    // ========================================================

    if (IsHoldingPickupThrow() &&
        ActiveInteractionTarget &&
        GetOwner())
    {
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
    }

    if (GunState == ESizeShiftGunState::Interacting &&
        PhysicsHandle &&
        PhysicsHandle->GetGrabbedComponent())
    {
        PhysicsHandle->SetTargetLocationAndRotation(
            GetHoldLocation(),
            HeldBoxRotation
        );
    }

    // ========================================================
    // PUSH / PULL
    // ========================================================

    if (GunState == ESizeShiftGunState::Interacting &&
        ActiveInteractionTarget &&
        ActiveInteractionTarget->GetInteractionType()
        == EBoxInteractionType::PushPull)
    {
        UpdatePushPull(DeltaTime);
    }
}

// ============================================================
// Aim
// ============================================================

ASizeShiftBox* ASizeShiftGun::GetAimTarget() const
{
    if (!GetOwner())
    {
        return nullptr;
    }

    AActor* OwnerActor = GetOwner();

    APlayerController* PlayerController =
        Cast<APlayerController>(
            OwnerActor->GetInstigatorController()
        );

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
    Params.AddIgnoredActor(OwnerActor);

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

        bInteractionStarted = StartPhysicsHold();

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
        SetGunState(
            ESizeShiftGunState::Idle
        );

        return;
    }

    const EBoxInteractionType InteractionType =
        ActiveInteractionTarget->GetInteractionType();

    switch (InteractionType)
    {
    case EBoxInteractionType::PickupThrow:

        StopPhysicsHold();

        break;

    case EBoxInteractionType::PushPull:

        StopPushPull();

        break;

    default:

        break;
    }

    ActiveInteractionTarget = nullptr;

    SetGunState(
        ESizeShiftGunState::Idle
    );
}

void ASizeShiftGun::SetGunState(
    ESizeShiftGunState NewState)
{
    GunState = NewState;

    PrintGunStatus(
        FString::Printf(
            TEXT("STATE = %s"),
            *GetGunStateName()
        )
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

// ============================================================
// Pickup / Throw
// ============================================================

FVector ASizeShiftGun::GetHoldLocation() const
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }

    const AActor* OwnerActor = GetOwner();

    const APlayerController* PlayerController =
        Cast<APlayerController>(
            OwnerActor->GetInstigatorController()
        );

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

    FVector HoldLocation =
        CameraLocation +
        CameraRotation.Vector() * HoldDistance;

    return HoldLocation;
}

bool ASizeShiftGun::StartPhysicsHold()
{
    if (!ActiveInteractionTarget)
    {
        return false;
    }

    if (!PhysicsHandle)
    {
        return false;
    }

    UPrimitiveComponent* PrimitiveComponent =
        ActiveInteractionTarget->GetBoxMesh();

    if (!PrimitiveComponent)
    {
        return false;
    }

    if (!PrimitiveComponent->IsSimulatingPhysics())
    {
        PrintGunStatus(
            TEXT("PHYSICS HOLD FAILED -> Physics Disabled")
        );

        return false;
    }  

    PrimitiveComponent->WakeAllRigidBodies();

    const FVector GrabLocation =
        ActiveInteractionTarget->GetActorLocation();

    HeldBoxRotation =
        PrimitiveComponent->GetComponentRotation();

    PhysicsHandle->GrabComponentAtLocationWithRotation(
        PrimitiveComponent,
        NAME_None,
        GrabLocation,
        HeldBoxRotation
    );

    PhysicsHandle->SetTargetLocation(
        GetHoldLocation()
    );

    return true;
}

void ASizeShiftGun::StopPhysicsHold()
{
    if (!PhysicsHandle)
    {
        return;
    }

    UPrimitiveComponent* GrabbedComponent =
        PhysicsHandle->GetGrabbedComponent();

    if (!GrabbedComponent)
    {
        return;
    }

    HeldBoxRotation = FRotator::ZeroRotator;

    PhysicsHandle->ReleaseComponent();

    GrabbedComponent->WakeAllRigidBodies();

    PrintGunStatus(
        TEXT("PHYSICS RELEASE")
    );
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

    if (GunState != ESizeShiftGunState::Interacting)
    {
        return;
    }

    if (!ActiveInteractionTarget)
    {
        return;
    }

    if (!PhysicsHandle)
    {
        return;
    }

    UPrimitiveComponent* GrabbedComponent =
        PhysicsHandle->GetGrabbedComponent();

    if (!GrabbedComponent)
    {
        return;
    }

    APlayerController* PlayerController =
        Cast<APlayerController>(
            GetOwner()->GetInstigatorController()
        );

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

    PhysicsHandle->ReleaseComponent();

    HeldBoxRotation = FRotator::ZeroRotator;

    GrabbedComponent->SetPhysicsLinearVelocity(
        ThrowVelocity,
        false
    );

    GrabbedComponent->WakeAllRigidBodies();

    PrintGunStatus(
        FString::Printf(
            TEXT(
                "THROW -> %s | Speed: %.1f"
            ),
            *ActiveInteractionTarget->GetName(),
            ThrowSpeed
        )
    );

    ActiveInteractionTarget = nullptr;

    SetGunState(
        ESizeShiftGunState::Idle
    );
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
    if (!ActiveInteractionTarget ||
        !GetOwner())
    {
        return;
    }

    if (ActiveInteractionTarget->GetInteractionType()
        != EBoxInteractionType::PushPull)
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

bool ASizeShiftGun::IsInteractingPushPull() const
{
    return GunState == ESizeShiftGunState::Interacting &&
        ActiveInteractionTarget &&
        ActiveInteractionTarget->GetInteractionType() ==
        EBoxInteractionType::PushPull;
}

void ASizeShiftGun::PushPullBurst()
{
    if (!IsInteractingPushPull() || !GetOwner())
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

    // Stop Push/Pull before launching.
    StopPushPull();

    ActiveInteractionTarget = nullptr;

    SetGunState(
        ESizeShiftGunState::Idle
    );

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

    HoldDistance = DefaultHoldDistance;

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
    bIsThrowAimMode = bNewThrowAimMode;

    if (bIsThrowAimMode)
    {
        HoldDistance = DefaultHoldDistance - 100.0f;

        ShowHeldBoxPreview();
    }
    else
    {
        HideHeldBoxPreview();
    }
}

//=============================================================

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

    FString TargetName = TEXT("None");

    if (ActiveInteractionTarget)
    {
        TargetName = ActiveInteractionTarget->GetName();
    }

    const FString StateName = GetGunStateName();

    const FString DebugMessage =
        FString::Printf(
            TEXT("[SizeShiftGun] %s | State: %s | Target: %s"),
            *Message,
            *StateName,
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




