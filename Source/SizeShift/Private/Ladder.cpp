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

    LadderMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(
        TEXT("LadderMesh")
    );
    RootComponent = LadderMesh;

    ClimbVolume = CreateDefaultSubobject<UBoxComponent>(
        TEXT("ClimbVolume")
    );
    ClimbVolume->SetupAttachment(RootComponent);

    ClimbVolume->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void ALadder::BeginPlay()
{
	Super::BeginPlay();

    if (ClimbVolume)
    {
        ClimbVolume->OnComponentBeginOverlap.AddDynamic(
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

    if (!LadderMesh)
    {
        return;
    }

    LadderMesh->ClearInstances();

    LadderMesh->SetStaticMesh(LadderMeshAsset);

    for (int32 Index = 0; Index < LadderSegments; ++Index)
    {
        const FVector Location(
            0.0f,
            0.0f,
            Index * SegmentHeight
        );

        const FTransform InstanceTransform(
            FRotator::ZeroRotator,
            Location,
            FVector::OneVector
        );

        LadderMesh->AddInstance(InstanceTransform);
    }

    const float LadderHeight = LadderSegments * SegmentHeight;

    if (ClimbVolume)
    {
        ClimbVolume->SetRelativeLocation(
            FVector(
                -40.0f,
                45.0f,
                LadderHeight / 2.0f
            )
        );

        ClimbVolume->SetBoxExtent(
            FVector(
                25.0f,
                60.0f,
                LadderHeight / 2.0f
            )
        );
    }
}

void ALadder::OnClimbVolumeBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!OtherActor)
    {
        return;
    }

    ASizeShiftCharacter* PlayerCharacter =
        Cast<ASizeShiftCharacter>(OtherActor);

    if (!PlayerCharacter)
    {
        return;
    }

    PlayerCharacter->StartClimbing(this);
}

float ALadder::GetLadderHeight() const
{
    return LadderSegments * SegmentHeight;
}

FVector ALadder::GetTopExitLocation() const
{
    const float LadderHeight = GetLadderHeight();

    return GetActorTransform().TransformPosition(
        FVector(0.0f, 0.0f, LadderHeight)
    );
}

FVector ALadder::GetBottomExitLocation() const
{
    return GetActorTransform().TransformPosition(
        FVector(0.0f, 0.0f, 0.0f)
    );
}


