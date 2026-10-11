#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SizeShiftCheckpoint.generated.h"

class UArrowComponent;
class UBoxComponent;

UCLASS()
class SIZESHIFT_API ASizeShiftCheckpoint : public AActor
{
    GENERATED_BODY()

public:
    ASizeShiftCheckpoint();

    UFUNCTION(BlueprintPure, Category = "Checkpoint")
    FVector GetRespawnLocation() const;

    UFUNCTION(BlueprintPure, Category = "Checkpoint")
    FRotator GetRespawnRotation() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
    USceneComponent* _Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
    UArrowComponent* _RespawnPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
    UBoxComponent* _Trigger;

    UFUNCTION()
    void OnCheckpointBeginOverlap(
        UPrimitiveComponent* overlappedComponent,
        AActor* otherActor,
        UPrimitiveComponent* otherComp,
        int32 otherBodyIndex,
        bool fromSweep,
        const FHitResult& sweepResult
    );
};