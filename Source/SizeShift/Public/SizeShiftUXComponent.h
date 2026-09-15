// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SizeShiftUXComponent.generated.h"

class ASizeShiftBox;
class UMaterialInterface;
class USizeShiftPromptWidget;
class USoundBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIZESHIFT_API USizeShiftUXComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	USizeShiftUXComponent();

	void UpdateTargetOutline(
		ASizeShiftBox* NewTarget,
		bool bCanInteract,
		bool bCanSizeShift,
		bool bIsHolding
	);

	void SetPromptState(
		bool bShowInteract,
		bool bShowSizeUp,
		bool bShowSizeDown
	);

	void SetPromptPosition(FVector2D ScreenPosition);

	void PlaySizeShiftSound();
	void PlayInteractSound();
	void PlayThrowSound();
	void PlayInvalidActionSound();

protected:
	
	virtual void BeginPlay() override;


private:

	void ClearCurrentOutline();

	void ApplyOutlineMaterial(UMaterialInterface* NewMaterial);

private:

	ASizeShiftBox* CurrentOutlineTarget = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Outline")
	UMaterialInterface* NormalOutlineMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Outline")
	UMaterialInterface* InteractOutlineMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Outline")
	UMaterialInterface* SizeShiftOutlineMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Outline")
	UMaterialInterface* HoldingOutlineMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Prompt")
	TSubclassOf<USizeShiftPromptWidget> PromptWidgetClass;

	UPROPERTY()
	USizeShiftPromptWidget* PromptWidget = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|SFX")
	USoundBase* SizeShiftSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|SFX")
	USoundBase* InteractSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|SFX")
	USoundBase* ThrowSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|SFX")
	USoundBase* InvalidActionSound = nullptr;

};
