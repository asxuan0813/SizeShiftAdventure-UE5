#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BoxMaterialData.h"
#include "BoxMaterialDataAsset.generated.h"

UCLASS(BlueprintType)
class SIZESHIFT_API UBoxMaterialDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
    FName _MaterialName = TEXT("Default");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
    F_BoxMaterialData _MaterialProperties;
};