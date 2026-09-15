#include "SizeShiftCheckpoint.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "SizeShiftCharacter.h"

ASizeShiftCheckpoint::ASizeShiftCheckpoint()
{
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    RespawnPoint =
        CreateDefaultSubobject<UArrowComponent>(TEXT("RespawnPoint"));

    RespawnPoint->SetupAttachment(Root);

    Trigger =
        CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));

    Trigger->SetupAttachment(Root);

    Trigger->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly
    );

    Trigger->SetCollisionResponseToAllChannels(
        ECR_Ignore
    );

    Trigger->SetCollisionResponseToChannel(
        ECC_Pawn,
        ECR_Overlap
    );

    Trigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &ASizeShiftCheckpoint::OnCheckpointBeginOverlap
    );
}

FVector ASizeShiftCheckpoint::GetRespawnLocation() const
{
    return RespawnPoint->GetComponentLocation();
}

FRotator ASizeShiftCheckpoint::GetRespawnRotation() const
{
    return RespawnPoint->GetComponentRotation();
}

void ASizeShiftCheckpoint::OnCheckpointBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    ASizeShiftCharacter* Character =
        Cast<ASizeShiftCharacter>(OtherActor);

    if (!Character)
    {
        return;
    }

    Character->SetCurrentCheckpoint(this);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Checkpoint] Player reached checkpoint")
    );
}

