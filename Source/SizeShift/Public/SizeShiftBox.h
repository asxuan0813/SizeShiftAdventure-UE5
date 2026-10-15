// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxMaterialDataAsset.h"
#include "SizeShiftBox.generated.h"

class UStaticMeshComponent;

// Represents the three sizes that determine a box's scale and physical properties
UENUM(BlueprintType)
enum class E_BoxSizeType : uint8
{
    Small,
    Medium,
    Large
};

// Selects how the SizeShift gun can interact with a box
UENUM(BlueprintType)
enum class E_BoxInteractionType : uint8
{
    PickupThrow UMETA(DisplayName = "Pickup / Throw"),
    PushPull    UMETA(DisplayName = "Push / Pull"),
    Immovable   UMETA(DisplayName = "Immovable")
};

UCLASS()
class SIZESHIFT_API ASizeShiftBox : public AActor
{
    GENERATED_BODY()

public:

    ASizeShiftBox();

    // Applies a specific size after validating the requested resize
    UFUNCTION(BlueprintCallable, Category = "Box")
    void SetBoxSize(E_BoxSizeType newSize);

    // Attempts to increase the box by one size level
    UFUNCTION(BlueprintCallable, Category = "Box")
    void IncreaseBoxSize();

    // Attempts to decrease the box by one size level
    UFUNCTION(BlueprintCallable, Category = "Box")
    void DecreaseBoxSize();

    // Returns the box's current size level
    UFUNCTION(BlueprintPure, Category = "Box")
    E_BoxSizeType GetBoxSize() const;

    // Returns the interaction mode derived from the box's properties
    UFUNCTION(BlueprintPure, Category = "Box")
    E_BoxInteractionType GetInteractionType() const;

    // Returns the calculated volume for the current size
    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    float GetVolume() const;

    // Returns the density resolved from the configured material
    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    float GetDensity() const;

    // Returns the calculated physical mass
    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    float GetMass() const;

    // Provides the mesh used for collision, physics, and visual updates
    UStaticMeshComponent* GetBoxMesh() const
    {
        return _BoxMesh;
    }

    // Checks whether the box has ground support for push/pull movement
    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    bool HasPushPullGroundSupport() const;

protected:

    virtual void BeginPlay() override;

    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
    UStaticMeshComponent* _BoxMesh;

    // Designer-selected initial size
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Box")
    E_BoxSizeType _SizeType = E_BoxSizeType::Small;

    // Material data used to resolve density and related box properties
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Box|Material")
    TObjectPtr<UBoxMaterialDataAsset> _MaterialData;

    // Volume before the current size multiplier is applied
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Box|Physics")
    float _BaseVolume = 1.0f;

    // Runtime interaction mode calculated from the current box properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
    E_BoxInteractionType _InteractionType =
        E_BoxInteractionType::PickupThrow;

    // Calculated Physics Values (Read Only)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float _Volume = 0.125f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float _Density = 500.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float _Mass = 25.0f;

    // Horizontal impulse applied when growing from Small to Medium
    UPROPERTY(EditAnywhere, Category = "Knockback")
    float _SmallToMediumHorizontalImpulse = 500.0f;

    // Vertical impulse applied when growing from Small to Medium
    UPROPERTY(EditAnywhere, Category = "Knockback")
    float _SmallToMediumVerticalImpulse = 150.0f;

    // Horizontal impulse applied when growing from Medium to Large
    UPROPERTY(EditAnywhere, Category = "Knockback")
    float _MediumToLargeHorizontalImpulse = 900.0f;

    // Vertical impulse applied when growing from Medium to Large
    UPROPERTY(EditAnywhere, Category = "Knockback")
    float _MediumToLargeVerticalImpulse = 250.0f;

private:

    // Recalculates volume, density, mass, mesh scale, and interaction data after resizing
    void UpdateBoxProperties();

    // Chooses the allowed interaction mode from the current box state
    void UpdateInteractionType();

    // Returns the scale multiplier for the current size
    float GetCurrentSizeMultiplier() const;

    // Returns the scale multiplier for a supplied size level
    float GetSizeMultiplier(E_BoxSizeType inSize) const;

    // Reads density from material data, falling back when no data is available
    float ResolveMaterialDensity() const;

    // Checks whether the requested target size has enough space to be applied
    bool CanResizeTo(E_BoxSizeType newSize) const;

    // Tests clearance in a direction before a size increase occupies additional space
    bool CanResizeInDirection(
        const FVector& direction,
        float requiredDistance
    ) const;

    // Applies the configured physical impulse when the box grows
    void ApplyGrowImpact();

    void PrintBoxStatus() const;
};
