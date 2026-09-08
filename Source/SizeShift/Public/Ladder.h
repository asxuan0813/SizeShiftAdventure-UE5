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

private:

	UPROPERTY(VisibleAnywhere, Category = "Ladder")
	TObjectPtr<UInstancedStaticMeshComponent> LadderMesh;

	UPROPERTY(VisibleAnywhere, Category = "Ladder")
	TObjectPtr<UBoxComponent> ClimbVolume;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	TObjectPtr<UStaticMesh> LadderMeshAsset;

	UPROPERTY(EditAnywhere, Category = "Ladder", meta = (ClampMin = "1"))
	int32 LadderSegments = 1;

	UPROPERTY(EditAnywhere, Category = "Ladder")
	float SegmentHeight = 200.0f;

	float ClimbingMinZ = 0.0f;
	float ClimbingMaxZ = 0.0f;

	UFUNCTION()
	void OnClimbVolumeBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

};
