// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoxMaterialDataAsset.h"
#include "SizeShiftBox.generated.h"

class UStaticMeshComponent;
class UPhysicalMaterial;

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

    // ========================================
    // SIZE
    // ========================================
    UFUNCTION(BlueprintCallable, Category = "Box")
    void SetBoxSize(EBoxSizeType NewSize);

    UFUNCTION(BlueprintCallable, Category = "Box")
    void IncreaseBoxSize();

    UFUNCTION(BlueprintCallable, Category = "Box")
    void DecreaseBoxSize();

    UFUNCTION(BlueprintPure, Category = "Box")
    EBoxSizeType GetBoxSize() const;

    UFUNCTION(BlueprintPure, Category = "Box")
    EBoxInteractionType GetInteractionType() const;

    // ========================================
    // PHYSICS
    // ========================================

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

    virtual void BeginPlay() override;

    // ========================================
    // COMPONENT
    // ========================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
    UStaticMeshComponent* BoxMesh;

    // ========================================
    // SIZE
    // ========================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Box")
    EBoxSizeType SizeType = EBoxSizeType::Small;

    // ========================================
    // INTERACTION
    // ========================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box")
    EBoxInteractionType InteractionType =
        EBoxInteractionType::PickupThrow;

    // ========================================
    // MATERIAL DATA
    // ========================================

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Box|Material")
    TObjectPtr<UBoxMaterialDataAsset> MaterialData;

    // ========================================
    // PHYSICS DATA
    // ========================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float Volume = 0.125f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float Density = 200.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Box|Physics")
    float Mass = 25.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Box|Physics")
    float BaseVolume = 1.0f;

    // ========================================
    // INTERNAL
    // ========================================

    void UpdateBoxProperties();

    void UpdateInteractionType();

    float GetSizeMultiplier() const;

    void UpdatePhysicsMaterial();

    void PrintBoxStatus() const;

};
