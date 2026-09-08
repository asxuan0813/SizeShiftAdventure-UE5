// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SizeShiftCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class ASizeShiftGun;
class UUserWidget;
class UChildActorComponent;
class ALadder;

UENUM(BlueprintType)
enum class EPlayerMovementState : uint8
{
	Normal,
	Sprinting,
	Crouching,
	Climbing
};

UCLASS()
class SIZESHIFT_API ASizeShiftCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASizeShiftCharacter();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void StartClimbing(ALadder* Ladder);

	void StopClimbing();

	void UpdateClimbingMovement(float InputZ);

protected:

	void Move(const struct FInputActionValue& Value);

	void Look(const struct FInputActionValue& Value);

	void Jump();

	void ToggleCrouch();

	void IncreaseSize();

	void DecreaseSize();

	void Interact();

	void StartSprint();

	void StopSprint();

	void SetMovementState(EPlayerMovementState NewState);

	EPlayerMovementState GetMovementState() const;

	void StopClimbingMovement();

	void ExitLadderTop();

	void ExitLadderBottom();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SizeUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SizeDownAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AdjustHoldDistanceAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* HoldBoxAsideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
	UChildActorComponent* SizeShiftGunComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun")
	TSubclassOf<ASizeShiftGun> SizeShiftGunClass;

	ASizeShiftGun* SizeShiftGun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY()
	UUserWidget* CrosshairWidget;

	FVector OriginalCameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EPlayerMovementState MovementState = EPlayerMovementState::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed = 900.0f;

	float OriginalWalkSpeed;

	UPROPERTY()
	ALadder* CurrentLadder = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float ClimbSpeed = 300.0f;

	UPROPERTY()
	TEnumAsByte<EMovementMode> OriginalMovementMode = MOVE_Walking;

	float OriginalGravityScale = 1.0f;

	void AdjustHoldDistance(
		const struct FInputActionValue& Value
	);

	void ToggleThrowAimMode();

	
};
