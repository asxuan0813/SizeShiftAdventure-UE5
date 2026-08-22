// Fill out your copyright notice in the Description page of Project Settings.


#include "SizeShiftGun.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "SizeShiftBox.h"
#include "SizeShiftCharacter.h"
#include "GameFramework/Character.h"

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

    PrintGunStatus(
        TEXT("Gun BeginPlay")
    );
	
}

// Called every frame
void ASizeShiftGun::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAimTarget();

    // ========================================================
    // PICKUP / THROW
    // ========================================================

    if (GunState == ESizeShiftGunState::Interacting &&
        PhysicsHandle &&
        PhysicsHandle->GetGrabbedComponent())
    {
        PhysicsHandle->SetTargetLocation(
            GetHoldLocation()
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
        return;
    }

    SetGunState(
        ESizeShiftGunState::Interacting
    );

    const EBoxInteractionType InteractionType =
        ActiveInteractionTarget->GetInteractionType();

    switch (InteractionType)
    {
    case EBoxInteractionType::PickupThrow:

        PrintGunStatus(
            TEXT("INTERACT -> PICKUP / THROW")
        );

        StartPhysicsHold();

        break;


    case EBoxInteractionType::PushPull:

        PrintGunStatus(
            TEXT("INTERACT -> PUSH / PULL")
        );

        StartPushPull();

        break;


    case EBoxInteractionType::Immovable:

        PrintGunStatus(
            TEXT("INTERACT BLOCKED -> IMMOVABLE")
        );

        SetGunState(
            ESizeShiftGunState::Idle
        );

        ActiveInteractionTarget = nullptr;

        break;


    default:

        SetGunState(
            ESizeShiftGunState::Idle
        );

        ActiveInteractionTarget = nullptr;

        break;
    }
}

void ASizeShiftGun::StopInteraction()
{
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

        Drop();

        break;

    case EBoxInteractionType::PushPull:

        //StopPushPull();

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

    FString StateName;

    switch (GunState)
    {
    case ESizeShiftGunState::Idle:
        StateName = TEXT("IDLE");
        break;

    case ESizeShiftGunState::Interacting:
        StateName = TEXT("INTERACTING");
        break;

    default:
        StateName = TEXT("UNKNOWN");
        break;
    }

    PrintGunStatus(
        FString::Printf(
            TEXT("STATE = %s"),
            *StateName
        )
    );
}

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

    FString StateName;

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

    return CameraLocation +
        CameraRotation.Vector() * HoldDistance;
}

void ASizeShiftGun::StartPhysicsHold()
{
    if (!ActiveInteractionTarget)
    {
        return;
    }

    if (!PhysicsHandle)
    {
        return;
    }

    UPrimitiveComponent* PrimitiveComponent =
        ActiveInteractionTarget->GetBoxMesh();

    if (!PrimitiveComponent)
    {
        return;
    }

    if (!PrimitiveComponent->IsSimulatingPhysics())
    {
        PrintGunStatus(
            TEXT("PHYSICS HOLD FAILED -> Physics Disabled")
        );

        return;
    }

    const FVector GrabLocation =
        ActiveInteractionTarget->GetActorLocation();

    PhysicsHandle->GrabComponentAtLocation(
        PrimitiveComponent,
        NAME_None,
        GrabLocation
    );

    PhysicsHandle->SetTargetLocation(
        GetHoldLocation()
    );

    PrintGunStatus(
        FString::Printf(
            TEXT(
                "PHYSICS GRAB -> %s | Hold Distance: %.1f"
            ),
            *ActiveInteractionTarget->GetName(),
            HoldDistance
        )
    );
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

    PhysicsHandle->ReleaseComponent();

    GrabbedComponent->WakeAllRigidBodies();

    PrintGunStatus(
        TEXT("PHYSICS RELEASE")
    );
}

void ASizeShiftGun::Drop()
{
    if (!PhysicsHandle)
    {
        return;
    }

    if (!PhysicsHandle->GetGrabbedComponent())
    {
        return;
    }

    StopPhysicsHold();

    PrintGunStatus(
        TEXT("DROP")
    );
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

void ASizeShiftGun::StartPushPull()
{
    if (!ActiveInteractionTarget || !GetOwner())
    {
        PrintGunStatus(
            TEXT("PUSH/PULL START FAILED -> Missing Target or Owner")
        );

        return;
    }

    if (ActiveInteractionTarget->GetInteractionType()
        != EBoxInteractionType::PushPull)
    {
        PrintGunStatus(
            TEXT("PUSH/PULL START FAILED -> Wrong Interaction Type")
        );

        return;
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

        return;
    }

    InitialDirection.Normalize();

    // ========================================================
    // INITIALIZE SMOOTH DIRECTION
    // ========================================================

    SmoothedPushPullDirection =
        InitialDirection;

    bHasPushPullDirection = true;

    // ========================================================
    // START INTERACTION
    // ========================================================

    SetGunState(
        ESizeShiftGunState::Interacting
    );

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
}

void ASizeShiftGun::UpdatePushPull(float DeltaTime)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            210,
            0.1f,
            bHasPushPullDirection
            ? FColor::Green
            : FColor::Red,
            bHasPushPullDirection
            ? TEXT("[PushPull] UPDATE ACTIVE")
            : TEXT("[PushPull] BLOCKED - NO DIRECTION")
        );
    }

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
        + SmoothedPushPullDirection * PushPullDistance;

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

    ToAnchor.Normalize();

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
    
}

void ASizeShiftGun::StopPushPull()
{
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

    PushPullAnchorOffset =
        FVector::ZeroVector;

    SmoothedPushPullDirection =
        FVector::ZeroVector;

    bHasPushPullDirection = false;
}

FVector ASizeShiftGun::GetPushPullAnchorLocation() const
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector Forward =
        GetOwner()->GetActorForwardVector();

    Forward.Z = 0.0f;

    if (Forward.IsNearlyZero())
    {
        return GetOwner()->GetActorLocation();
    }

    Forward.Normalize();

    // The actual smoothing happens in UpdatePushPull().
    return GetOwner()->GetActorLocation()
        + Forward * PushPullDistance;
}


