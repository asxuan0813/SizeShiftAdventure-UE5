// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxMaterialDataAsset.h"
#include "SizeShiftBox.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class E_BoxSizeType : uint8
{
    Small,
    Medium,
    Large
};

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

    // ============================================================
    // Size Commands
    // ============================================================

    UFUNCTION(BlueprintCallable, Category = "Box")
    void SetBoxSize(E_BoxSizeType newSize);

    UFUNCTION(BlueprintCallable, Category = "Box")
    void IncreaseBoxSize();

    UFUNCTION(BlueprintCallable, Category = "Box")
    void DecreaseBoxSize();

    // ============================================================
    // Read-only Box Data
    // ============================================================

    UFUNCTION(BlueprintPure, Category = "Box")
    E_BoxSizeType GetBoxSize() const;

    UFUNCTION(BlueprintPure, Category = "Box")
    E_BoxInteractionType GetInteractionType() const;

    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    float GetVolume() const;

    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    float GetDensity() const;

    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    float GetMass() const;

    UStaticMeshComponent* GetBoxMesh() const
    {
        return _BoxMesh;
    }

    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    bool HasPushPullGroundSupport() const;


protected:

    // ============================================================
    // Life Cycle
    // ============================================================

    virtual void BeginPlay() override;

    virtual void OnConstruction(const FTransform& Transform) override;

    // ============================================================
    // Component
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
    UStaticMeshComponent* _BoxMesh;

    // ============================================================
    // Editable Configuration
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Box")
    E_BoxSizeType _SizeType = E_BoxSizeType::Small;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Box|Material")
    TObjectPtr<UBoxMaterialDataAsset> _MaterialData;

    // Physics Configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Box|Physics")
    float _BaseVolume = 1.0f;

    // ============================================================
    // Calculated Runtime Data
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
    E_BoxInteractionType _InteractionType =
        E_BoxInteractionType::PickupThrow;

    // Calculated Physics Values (Read Only)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float _Volume = 0.125f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float _Density = 200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float _Mass = 25.0f;

    // ============================================================
    // Knockback
    // ============================================================

    UPROPERTY(EditAnywhere, Category = "Knockback")
    float _SmallToMediumHorizontalImpulse = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Knockback")
    float _SmallToMediumVerticalImpulse = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Knockback")
    float _MediumToLargeHorizontalImpulse = 900.0f;

    UPROPERTY(EditAnywhere, Category = "Knockback")
    float _MediumToLargeVerticalImpulse = 250.0f;

private:

    // ============================================================
    // Internal Calculation
    // ============================================================

    void UpdateBoxProperties();

    void UpdateInteractionType();

    float GetCurrentSizeMultiplier() const;

    float GetSizeMultiplier(E_BoxSizeType inSize) const;

    float ResolveMaterialDensity() const;

    bool CanResizeTo(E_BoxSizeType newSize) const;

    bool CanResizeInDirection(
        const FVector& direction,
        float requiredDistance
    ) const;

    void ApplyGrowImpact();

    // ============================================================
    // DEBUG
    // ============================================================

    void PrintBoxStatus() const;
};
