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

// Player movement states to control movement behavior
UENUM(BlueprintType)
enum class E_PlayerMovementState : uint8
{
	Normal,
	Sprinting,
	Crouching,
	Climbing // Ladder
};

// Main first person character: coordinates input, movement, intearaction and respawning
UCLASS()
class SIZESHIFT_API ASizeShiftCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ASizeShiftCharacter();

	// Update the highest vertical position while tracking highest fall
	virtual void Tick(float DeltaSeconds) override;

protected:

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

public:	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Enters ladder climbing and adjusts movement settings
	void StartClimbing(ALadder* ladder);

	// Leaves ladder climbing and restores movement settings
	void StopClimbing();

	// Applies vertical climb speed and checks ladder exit boundaries
	void UpdateClimbingMovement(float inputZ);

	// Returns the SizeShift Box currently supporting the character
	UFUNCTION(BlueprintPure, Category = "Movement")
	ASizeShiftBox* GetStandingBox() const;

	// Show the character health component
	UFUNCTION(BlueprintPure, Category = "Health")
	UHealthComponent* GetHealthComponent() const;

	// Respond to the death event
	UFUNCTION()
	void HandleDeath();

	// Store the checkpoint used
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

	// Updates the custom movement state
	void SetMovementState(E_PlayerMovementState newState);

	// Return the current custom movement state
	E_PlayerMovementState GetMovementState() const;

	// Clears vertical velocity when climbing input ends
	void StopClimbingMovement();

	// Moves the character to the upper ladder exit
	void ExitLadderTop();

	// Moves the character to the lower ladder exit
	void ExitLadderBottom();

	// Records the landed box and finishes fall tracking
	virtual void Landed(const FHitResult& hit) override;

	// Starts fall tracking when movement changes to falling
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

	// Child actor component that hosts the SizeShift Gun
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
	UChildActorComponent* _SizeShiftGunComponent;

	// Gun class selected in the editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gun")
	TSubclassOf<ASizeShiftGun> _SizeShiftGunClass;

	// Runtime reference to the spawned SizeShift Gun
	ASizeShiftGun* _SizeShiftGun;

	// Widget class used to create the crosshair
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> _CrosshairWidgetClass;

	UPROPERTY()
	UUserWidget* _CrosshairWidget;

	// Original camera offset restored after crouching
	FVector _OriginalCameraLocation;

	// Custom state and movement related runtime data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	E_PlayerMovementState _MovementState = E_PlayerMovementState::Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	ASizeShiftBox* _StandingBox = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float _SprintSpeed = 900.0f;

	float _OriginalWalkSpeed;

	// Active ladder and settings saved for restoring normal movement
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

	// Begins recording the character's falling height
	void StartFallTracking();

	// Calculates fall distance and applies damage if unsafe
	void FinishFallTracking();

	// Handles health, damage, and death events
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	UHealthComponent* _HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Fall Damage")
	float _SafeFallDistance = 300.0f;

	// Forwards hold-distance input to the SizeShift Gun
	void AdjustHoldDistance(
		const struct FInputActionValue& value
	);

	// Toggles the gun's throw aiming mode
	void ToggleThrowAimMode();

	// Initial spawn transform used when no checkpoint is active
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
    FVector _RespawnLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
    FRotator _RespawnRotation;

	// Restores position, stops movement, and resets health
    void Respawn();

	// Most recently assigned checkpoint
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	ASizeShiftCheckpoint* _CurrentCheckpoint;

	// Provides character UX feedback such as the death sound
	UPROPERTY(VisibleAnywhere)
	USizeShiftUXComponent* _UXComponent;
};
