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
class ASizeShiftBox;
class UHealthComponent;
class ASizeShiftCheckpoint;
class USizeShiftUXComponent;

UENUM(BlueprintType)
enum class E_PlayerMovementState : uint8
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

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void StartClimbing(ALadder* ladder);

	void StopClimbing();

	void UpdateClimbingMovement(float inputZ);

	UFUNCTION(BlueprintPure, Category = "Movement")
	ASizeShiftBox* GetStandingBox() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	UHealthComponent* GetHealthComponent() const;

	UFUNCTION()
	void HandleDeath();

	void SetCurrentCheckpoint(ASizeShiftCheckpoint* checkpoint);

protected:

	void Move(const struct FInputActionValue& value);

	void Look(const struct FInputActionValue& value);

	void Jump();

	void ToggleCrouch();

	void IncreaseSize();

	void DecreaseSize();

	void Interact();

	void StartSprint();

	void StopSprint();

	void SetMovementState(E_PlayerMovementState newState);

	E_PlayerMovementState GetMovementState() const;

	void StopClimbingMovement();

	void ExitLadderTop();

	void ExitLadderBottom();

	virtual void Landed(const FHitResult& hit) override;

	virtual void OnMovementModeChanged(
		EMovementMode prevMovementMode,
		uint8 previousCustomMode
	) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* _FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _SizeUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _SizeDownAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _AdjustHoldDistanceAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* _HoldBoxAsideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* _DefaultMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
	UChildActorComponent* _SizeShiftGunComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun")
	TSubclassOf<ASizeShiftGun> _SizeShiftGunClass;

	ASizeShiftGun* _SizeShiftGun;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> _CrosshairWidgetClass;

	UPROPERTY()
	UUserWidget* _CrosshairWidget;

	FVector _OriginalCameraLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	E_PlayerMovementState _MovementState = E_PlayerMovementState::Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	ASizeShiftBox* _StandingBox = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float _SprintSpeed = 900.0f;

	float _OriginalWalkSpeed;

	UPROPERTY()
	ALadder* _CurrentLadder = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float _ClimbSpeed = 300.0f;

	UPROPERTY()
	TEnumAsByte<EMovementMode> _OriginalMovementMode = MOVE_Walking;

	float _OriginalGravityScale = 1.0f;

	// Fall Detection
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Fall")
	bool _IsTrackingFall = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Fall")
	float _FallStartZ = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Fall")
	float _LastFallDistance = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Fall")
	float _HighestFallZ = 0.0f;

	void StartFallTracking();
	void FinishFallTracking();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	UHealthComponent* _HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Fall Damage")
	float _SafeFallDistance = 300.0f;

	void AdjustHoldDistance(
		const struct FInputActionValue& value
	);

	void ToggleThrowAimMode();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
    FVector _RespawnLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
    FRotator _RespawnRotation;

    void Respawn();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	ASizeShiftCheckpoint* _CurrentCheckpoint;

	UPROPERTY(VisibleAnywhere)
	USizeShiftUXComponent* _UXComponent;
};
