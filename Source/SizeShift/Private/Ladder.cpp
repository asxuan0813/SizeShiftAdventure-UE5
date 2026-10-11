// Fill out your copyright notice in the Description page of Project Settings.


#include "Ladder.h"
#include "SizeShiftCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ALadder::ALadder()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    _LadderMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
        TEXT("LadderMesh")
    );
    RootComponent = _LadderMesh;

    _ClimbVolume = CreateDefaultSubobject<UBoxComponent>(
        TEXT("ClimbVolume")
    );
    _ClimbVolume->SetupAttachment(RootComponent);

    _ClimbVolume->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ALadder::BeginPlay()
{
	Super::BeginPlay();

    if (_ClimbVolume)
    {
        _ClimbVolume->OnComponentBeginOverlap.AddDynamic(
            this,
            &ALadder::OnClimbVolumeBeginOverlap
        );
    }
	
}

// Called every frame
void ALadder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALadder::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (!_LadderMesh)
    {
        return;
    }

    _LadderMesh->ClearInstances();

    _LadderMesh->SetStaticMesh(_LadderMeshAsset);

    for (int32 index = 0; index < _LadderSegments; ++index)
    {
        const FVector location(
            0.0f,
            0.0f,
            index * _SegmentHeight
        );

        const FTransform instanceTransform(
            FRotator::ZeroRotator,
            location,
            FVector::OneVector
        );

        _LadderMesh->AddInstance(instanceTransform);
    }

    const float ladderHeight = _LadderSegments * _SegmentHeight;

    if (_ClimbVolume)
    {
        _ClimbVolume->SetRelativeLocation(
            FVector(
                _ClimbVolumeOffsetX,
                _ClimbVolumeOffsetY,
                ladderHeight / 2.0f
            )
        );

        _ClimbVolume->SetBoxExtent(
            FVector(
                _ClimbVolumeExtentX,
                _ClimbVolumeExtentY,
                ladderHeight / 2.0f
            )
        );
    }
}

void ALadder::OnClimbVolumeBeginOverlap(
    UPrimitiveComponent* overlappedComponent,
    AActor* otherActor,
    UPrimitiveComponent* otherComp,
    int32 otherBodyIndex,
    bool fromSweep,
    const FHitResult& sweepResult
)
{
    if (!otherActor)
    {
        return;
    }

    ASizeShiftCharacter* playerCharacter =
        Cast<ASizeShiftCharacter>(otherActor);

    if (!playerCharacter)
    {
        return;
    }

    playerCharacter->StartClimbing(this);
}

float ALadder::GetLadderHeight() const
{
    return _LadderSegments * _SegmentHeight;
}

FVector ALadder::GetTopExitLocation() const
{
    const float ladderHeight = GetLadderHeight();

    return GetActorTransform().TransformPosition(
        FVector(0.0f, 0.0f, ladderHeight)
    );
}

FVector ALadder::GetBottomExitLocation() const
{
    return GetActorTransform().TransformPosition(
        FVector(0.0f, 0.0f, 0.0f)
    );
}
