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
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
    UArrowComponent* RespawnPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
    UBoxComponent* Trigger;

    UFUNCTION()
    void OnCheckpointBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );
};