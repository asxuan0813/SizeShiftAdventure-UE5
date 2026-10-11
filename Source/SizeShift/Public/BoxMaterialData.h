#pragma once

#include "CoreMinimal.h"
#include "BoxMaterialData.generated.h"

USTRUCT(BlueprintType)
struct F_BoxMaterialData
{
    GENERATED_BODY()

public:

    // ========================================
    // PHYSICS
    // ========================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float _Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float _Friction = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float _Restitution = 0.2f;

    // ========================================
    // GAMEPLAY
    // ========================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    float _PushResistance = 1.0f;

    // ========================================
    // FUTURE
    // ========================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool _CanBreak = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    bool _CanConductLaser = false;
};
