#pragma once

#include "CoreMinimal.h"
#include "BoxMaterialData.generated.h"

USTRUCT(BlueprintType)
struct FBoxMaterialData
{
    GENERATED_BODY()

public:

    // ========================================
    // PHYSICS
    // ========================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float Friction = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float Restitution = 0.2f;

    // ========================================
    // GAMEPLAY
    // ========================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    float PushResistance = 1.0f;

    // ========================================
    // FUTURE
    // ========================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCanBreak = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool bCanConductLaser = false;
};
