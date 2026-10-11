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
		ASizeShiftBox* newTarget,
		bool canInteract,
		bool canSizeShift,
		bool isHolding
	);

	void SetPromptState(
		bool showInteract,
		bool showSizeUp,
		bool showSizeDown
	);

	void SetPromptPosition(FVector2D screenPosition);

	void PlaySizeShiftSound();
	void PlayInteractSound();
	void PlayThrowSound();
	void PlayInvalidActionSound();
	void PlayDeathSound();

protected:
	
	virtual void BeginPlay() override;


private:

	void ClearCurrentOutline();

	void ApplyOutlineMaterial(UMaterialInterface* newMaterial);

private:

	ASizeShiftBox* _CurrentOutlineTarget = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Outline")
	UMaterialInterface* _NormalOutlineMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Outline")
	UMaterialInterface* _InteractOutlineMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Outline")
	UMaterialInterface* _SizeShiftOutlineMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Outline")
	UMaterialInterface* _HoldingOutlineMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|Prompt")
	TSubclassOf<USizeShiftPromptWidget> _PromptWidgetClass;

	UPROPERTY()
	USizeShiftPromptWidget* _PromptWidget = nullptr;

	// SFX

	UPROPERTY(EditAnywhere, Category = "UX|SFX")
	USoundBase* _SizeShiftSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|SFX")
	USoundBase* _InteractSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|SFX")
	USoundBase* _ThrowSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|SFX")
	USoundBase* _InvalidActionSound = nullptr;

	UPROPERTY(EditAnywhere, Category = "UX|SFX")
	USoundBase* _DeathSound = nullptr;

};
