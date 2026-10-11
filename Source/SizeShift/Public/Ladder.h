// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ladder.generated.h"

class UBoxComponent;
class UInstancedStaticMeshComponent;
class UStaticMesh;

UCLASS()
class SIZESHIFT_API ALadder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALadder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;

	float GetLadderHeight() const;

	FVector GetTopExitLocation() const;
	FVector GetBottomExitLocation() const;

	UPROPERTY(EditAnywhere, Category = "Ladder|Climb Volume")
	float _ClimbVolumeOffsetX = -40.0f;

	UPROPERTY(EditAnywhere, Category = "Ladder|Climb Volume")
	float _ClimbVolumeOffsetY = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Ladder|Climb Volume")
	float _ClimbVolumeExtentX = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Ladder|Climb Volume")
	float _ClimbVolumeExtentY = 60.0f;

private:

	UPROPERTY(VisibleAnywhere, Category = "Ladder")
	TObjectPtr<UInstancedStaticMeshComponent> _LadderMesh;

	UPROPERTY(VisibleAnywhere, Category = "Ladder")
	TObjectPtr<UBoxComponent> _ClimbVolume;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	TObjectPtr<UStaticMesh> _LadderMeshAsset;

	UPROPERTY(EditAnywhere, Category = "Ladder", meta = (ClampMin = "1"))
	int32 _LadderSegments = 1;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	float _SegmentHeight = 200.0f;

	float _ClimbingMinZ = 0.0f;
	float _ClimbingMaxZ = 0.0f;

	UFUNCTION()
	void OnClimbVolumeBeginOverlap(
		UPrimitiveComponent* overlappedComponent,
		AActor* otherActor,
		UPrimitiveComponent* otherComp,
		int32 otherBodyIndex,
		bool fromSweep,
		const FHitResult& sweepResult
	);

};
