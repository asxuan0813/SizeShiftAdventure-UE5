// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxMaterialDataAsset.h"
#include "SizeShiftBox.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EBoxSizeType : uint8
{
    Small,
    Medium,
    Large
};

UENUM(BlueprintType)
enum class EBoxInteractionType : uint8
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
    void SetBoxSize(EBoxSizeType NewSize);

    UFUNCTION(BlueprintCallable, Category = "Box")
    void IncreaseBoxSize();

    UFUNCTION(BlueprintCallable, Category = "Box")
    void DecreaseBoxSize();

    // ============================================================
    // Read-only Box Data
    // ============================================================

    UFUNCTION(BlueprintPure, Category = "Box")
    EBoxSizeType GetBoxSize() const;

    UFUNCTION(BlueprintPure, Category = "Box")
    EBoxInteractionType GetInteractionType() const;

    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    float GetVolume() const;

    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    float GetDensity() const;

    UFUNCTION(BlueprintPure, Category = "Box|Physics")
    float GetMass() const;

    UStaticMeshComponent* GetBoxMesh() const
    {
        return BoxMesh;
    }

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
    UStaticMeshComponent* BoxMesh;

    // ============================================================
    // Editable Configuration
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Box")
    EBoxSizeType SizeType = EBoxSizeType::Small;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Box|Material")
    TObjectPtr<UBoxMaterialDataAsset> MaterialData;

    // Physics Configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Box|Physics")
    float BaseVolume = 1.0f;

    // ============================================================
    // Calculated Runtime Data
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
    EBoxInteractionType InteractionType =
        EBoxInteractionType::PickupThrow;

    // Calculated Physics Values (Read Only)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float Volume = 0.125f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float Density = 200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float Mass = 25.0f;

private:

    // ============================================================
    // Internal Calculation
    // ============================================================

    void UpdateBoxProperties();

    void UpdateInteractionType();

    float GetSizeMultiplier() const;

    float ResolveMaterialDensity() const;

    // ============================================================
    // DEBUG
    // ============================================================

    void PrintBoxStatus() const;

};
