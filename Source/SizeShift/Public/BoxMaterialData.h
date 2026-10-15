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
    float density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float friction = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    float restitution = 0.2f;
};
