#include "SizeShiftCheckpoint.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "SizeShiftCharacter.h"

ASizeShiftCheckpoint::ASizeShiftCheckpoint()
{
    PrimaryActorTick.bCanEverTick = false;

    _Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(_Root);

    _RespawnPoint =
        CreateDefaultSubobject<UArrowComponent>(TEXT("RespawnPoint"));

    _RespawnPoint->SetupAttachment(_Root);

    _Trigger =
        CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));

    _Trigger->SetupAttachment(_Root);

    _Trigger->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly
    );

    _Trigger->SetCollisionResponseToAllChannels(
        ECR_Ignore
    );

    _Trigger->SetCollisionResponseToChannel(
        ECC_Pawn,
        ECR_Overlap
    );

    _Trigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &ASizeShiftCheckpoint::OnCheckpointBeginOverlap
    );
}

FVector ASizeShiftCheckpoint::GetRespawnLocation() const
{
    return _RespawnPoint->GetComponentLocation();
}

FRotator ASizeShiftCheckpoint::GetRespawnRotation() const
{
    return _RespawnPoint->GetComponentRotation();
}

void ASizeShiftCheckpoint::OnCheckpointBeginOverlap(
    UPrimitiveComponent* overlappedComponent,
    AActor* otherActor,
    UPrimitiveComponent* otherComp,
    int32 otherBodyIndex,
    bool fromSweep,
    const FHitResult& sweepResult
)
{
    ASizeShiftCharacter* character =
        Cast<ASizeShiftCharacter>(otherActor);

    if (!character)
    {
        return;
    }

    character->SetCurrentCheckpoint(this);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Checkpoint] Player reached checkpoint")
    );
}

