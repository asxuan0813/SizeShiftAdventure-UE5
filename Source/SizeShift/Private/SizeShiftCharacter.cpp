// Fill out your copyright notice in the Description page of Project Settings.


#include "SizeShiftCharacter.h"
#include "Ladder.h"
#include "Components/ChildActorComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "SizeShiftGun.h"
#include "SizeShiftBox.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "HealthComponent.h"
#include "SizeShiftCheckpoint.h"
#include "SizeShiftUXComponent.h"

// Sets default values
ASizeShiftCharacter::ASizeShiftCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));

	_FirstPersonCamera->SetupAttachment(GetCapsuleComponent());

	_FirstPersonCamera->SetRelativeLocation(
		FVector(0.0f, 0.0f, 64.0f)
	);

	_FirstPersonCamera->bUsePawnControlRotation = true;

	_SizeShiftGunComponent =
		CreateDefaultSubobject<UChildActorComponent>(
			TEXT("SizeShiftGun")
		);

	_SizeShiftGunComponent->SetupAttachment(
		_FirstPersonCamera
	);

	_UXComponent = CreateDefaultSubobject<USizeShiftUXComponent>(
		TEXT("UXComponent")
	);

	_HealthComponent =
		CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	_CurrentCheckpoint = nullptr;

}

void ASizeShiftCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (_SizeShiftGunComponent && _SizeShiftGunClass)
	{
		_SizeShiftGunComponent->SetChildActorClass(
			_SizeShiftGunClass
		);
	}
}

void ASizeShiftCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!_IsTrackingFall)
	{
		return;
	}

	const float currentZ = GetActorLocation().Z;

	_HighestFallZ = FMath::Max(
		_HighestFallZ,
		currentZ
	);
}

// Called when the game starts or when spawned
void ASizeShiftCharacter::BeginPlay()
{
	Super::BeginPlay();

	_OriginalCameraLocation = _FirstPersonCamera->GetRelativeLocation();

	_OriginalWalkSpeed =
		GetCharacterMovement()->MaxWalkSpeed;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bEnablePhysicsInteraction = false;
	}

	if (APlayerController* playerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* localPlayer = playerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* subsystem =
				localPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (_DefaultMappingContext)
				{
					subsystem->AddMappingContext(_DefaultMappingContext, 0);
				}
			}
		}
	}

	if (_HealthComponent)
	{
		_HealthComponent->_OnDeath.AddDynamic(
			this,
			&ASizeShiftCharacter::HandleDeath
		);
	}

	if (_SizeShiftGunComponent)
	{
		_SizeShiftGun =
			Cast<ASizeShiftGun>(
				_SizeShiftGunComponent->GetChildActor()
			);
	}

	if (_SizeShiftGun)
	{
		_SizeShiftGun->SetOwner(this);
		_SizeShiftGun->SetInstigator(this);
	}

	if (_CrosshairWidgetClass)
	{
		_CrosshairWidget =
			CreateWidget<UUserWidget>(
				GetWorld(),
				_CrosshairWidgetClass
			);

		if (_CrosshairWidget)
		{
			_CrosshairWidget->AddToViewport();
		}
	}

	_RespawnLocation = GetActorLocation();
	_RespawnRotation = GetActorRotation();
}

void ASizeShiftCharacter::Move(const FInputActionValue& value)
{
	const FVector2D movementVector =
		value.Get<FVector2D>();

	if (_MovementState == E_PlayerMovementState::Climbing)
	{
		UpdateClimbingMovement(movementVector.Y);
		return;
	}

	AddMovementInput(
		GetActorForwardVector(),
		movementVector.Y
	);

	AddMovementInput(
		GetActorRightVector(),
		movementVector.X
	);
}

void ASizeShiftCharacter::Look(const FInputActionValue& value)
{
	FVector2D lookAxisVector = value.Get<FVector2D>();

	AddControllerYawInput(lookAxisVector.X);
	AddControllerPitchInput(lookAxisVector.Y);
}

void ASizeShiftCharacter::Jump()
{
	if (_MovementState == E_PlayerMovementState::Climbing)
	{
		return;
	}

	ACharacter::Jump();
}

void ASizeShiftCharacter::ToggleCrouch()
{
	if (_MovementState == E_PlayerMovementState::Climbing)
	{
		return;
	}

	if (_MovementState == E_PlayerMovementState::Sprinting)
	{
		StopSprint();
	}

	if (_MovementState == E_PlayerMovementState::Crouching)
	{
		UnCrouch();

		_FirstPersonCamera->SetRelativeLocation(
			_OriginalCameraLocation
		);

		SetMovementState(E_PlayerMovementState::Normal);
	}
	else
	{
		Crouch();

		FVector cameraLocation =
			_FirstPersonCamera->GetRelativeLocation();

		cameraLocation.Z = 40.0f;

		_FirstPersonCamera->SetRelativeLocation(
			cameraLocation
		);

		SetMovementState(E_PlayerMovementState::Crouching);
	}
}

void ASizeShiftCharacter::StartSprint()
{
	if (_MovementState == E_PlayerMovementState::Crouching)
	{
		UnCrouch();
		_FirstPersonCamera->SetRelativeLocation(_OriginalCameraLocation);
	}

	if (_MovementState == E_PlayerMovementState::Climbing)
	{
		return;
	}

	SetMovementState(E_PlayerMovementState::Sprinting);

	GetCharacterMovement()->MaxWalkSpeed = _SprintSpeed;
}

void ASizeShiftCharacter::StopSprint()
{
	if (_MovementState != E_PlayerMovementState::Sprinting)
	{
		return;
	}

	SetMovementState(E_PlayerMovementState::Normal);

	GetCharacterMovement()->MaxWalkSpeed = _OriginalWalkSpeed;
}

void ASizeShiftCharacter::StartClimbing(ALadder* ladder)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT(">>> START CLIMBING | Location: %s"),
		*GetActorLocation().ToString()
	);

	if (!ladder)
	{
		return;
	}

	if (_MovementState == E_PlayerMovementState::Climbing)
	{
		return;
	}

	if (_SizeShiftGun)
	{
		_SizeShiftGun->CancelInteraction();
	}

	_CurrentLadder = ladder;

	const FTransform ladderTransform =
		ladder->GetActorTransform();

	FVector localLocation =
		ladderTransform.InverseTransformPosition(
			GetActorLocation()
		);

	// Keep current height, only align horizontally.
	localLocation.X = -50.0f;
	localLocation.Y = 50.0f;

	const FVector alignedLocation =
		ladderTransform.TransformPosition(
			localLocation
		);

	SetActorLocation(
		alignedLocation,
		false
	);

	UCharacterMovementComponent* movement =
		GetCharacterMovement();

	_OriginalMovementMode = movement->MovementMode;
	_OriginalGravityScale = movement->GravityScale;

	// Stop any existing movement
	movement->Velocity = FVector::ZeroVector;

	// Cancel sprint / crouch
	if (_MovementState == E_PlayerMovementState::Sprinting)
	{
		movement->MaxWalkSpeed = _OriginalWalkSpeed;
	}

	if (_MovementState == E_PlayerMovementState::Crouching)
	{
		UnCrouch();

		_FirstPersonCamera->SetRelativeLocation(
			_OriginalCameraLocation
		);
	}

	SetMovementState(E_PlayerMovementState::Climbing);

	movement->GravityScale = 0.0f;
	movement->SetMovementMode(MOVE_Flying);
}

void ASizeShiftCharacter::StopClimbing()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT(">>> STOP CLIMBING | Location: %s"),
		*GetActorLocation().ToString()
	);

	if (_MovementState != E_PlayerMovementState::Climbing)
	{
		return;
	}


	UCharacterMovementComponent* movement =
		GetCharacterMovement();

	movement->Velocity = FVector::ZeroVector;

	movement->GravityScale = _OriginalGravityScale;
	movement->SetMovementMode(_OriginalMovementMode);

	_CurrentLadder = nullptr;

	SetMovementState(E_PlayerMovementState::Normal);
}

void ASizeShiftCharacter::StopClimbingMovement()
{
	if (_MovementState != E_PlayerMovementState::Climbing)
	{
		return;
	}

	FVector velocity = GetCharacterMovement()->Velocity;

	velocity.Z = 0.0f;

	GetCharacterMovement()->Velocity = velocity;
}

void ASizeShiftCharacter::UpdateClimbingMovement(float inputZ)
{
	if (!_CurrentLadder)
	{
		return;
	}

	const FTransform ladderTransform =
		_CurrentLadder->GetActorTransform();

	const FVector localLocation =
		ladderTransform.InverseTransformPosition(
			GetActorLocation()
		);

	const float ladderHeight =
		_CurrentLadder->GetLadderHeight();

	const float playerHalfHeight =
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float minZ = playerHalfHeight;

	const float bottomThreshold = 1.0f;
	const float topThreshold = 1.0f;

	const float maxZ = ladderHeight + playerHalfHeight;

	// Bottom
	if (localLocation.Z <= minZ + bottomThreshold &&
		inputZ < 0.0f)
	{
		ExitLadderBottom();
		return;
	}

	// Top
	if (localLocation.Z >= maxZ - topThreshold &&
		inputZ > 0.0f)
	{
		ExitLadderTop();
		return;
	}

	if (FMath::IsNearlyZero(inputZ))
	{
		GetCharacterMovement()->Velocity.Z = 0.0f;
		return;
	}

	FVector velocity =
		GetCharacterMovement()->Velocity;

	velocity.X = 0.0f;
	velocity.Y = 0.0f;
	velocity.Z = inputZ * _ClimbSpeed;

	GetCharacterMovement()->Velocity = velocity;
}

void ASizeShiftCharacter::ExitLadderTop()
{
	if (!_CurrentLadder)
	{
		return;
	}

	const FTransform ladderTransform =
		_CurrentLadder->GetActorTransform();

	const float ladderHeight =
		_CurrentLadder->GetLadderHeight();

	const float playerHalfHeight =
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// Move the player onto the top platform.
	const FVector localExitLocation(
		10.0f,
		50.0f,
		ladderHeight + playerHalfHeight
	);

	const FVector exitLocation =
		ladderTransform.TransformPosition(
			localExitLocation
		);

	SetActorLocation(
		exitLocation,
		false
	);

	StopClimbing();
}

void ASizeShiftCharacter::ExitLadderBottom()
{
	if (!_CurrentLadder)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("BOTTOM EXIT FAILED: CurrentLadder is NULL")
		);

		return;
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("BOTTOM EXIT FUNCTION CALLED")
	);

	const FTransform ladderTransform =
		_CurrentLadder->GetActorTransform();

	const float playerHalfHeight =
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FVector localExitLocation(
		-150.0f,
		50.0f,
		playerHalfHeight
	);

	const FVector exitLocation =
		ladderTransform.TransformPosition(
			localExitLocation
		);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("BOTTOM EXIT LOCATION: %s"),
		*exitLocation.ToString()
	);

	SetActorLocation(exitLocation, false);

	StopClimbing();
}

void ASizeShiftCharacter::Landed(const FHitResult& hit)
{
	Super::Landed(hit);

	ASizeShiftBox* landedBox =
		Cast<ASizeShiftBox>(hit.GetActor());

	_StandingBox = landedBox;

	if (_IsTrackingFall)
	{
		FinishFallTracking();
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[StandingBox] Landed | HitActor: %s | LandedBox: %s"),
		hit.GetActor()
		? *hit.GetActor()->GetName()
		: TEXT("NULL"),
		_StandingBox
		? *_StandingBox->GetName()
		: TEXT("NULL")
	);
}

void ASizeShiftCharacter::OnMovementModeChanged(
	EMovementMode prevMovementMode,
	uint8 previousCustomMode)
{
	Super::OnMovementModeChanged(
		prevMovementMode,
		previousCustomMode
	);

	if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		_StandingBox = nullptr;

		StartFallTracking();
	}
}

void ASizeShiftCharacter::StartFallTracking()
{
	if (_IsTrackingFall)
	{
		return;
	}

	_IsTrackingFall = true;

	_FallStartZ = GetActorLocation().Z;
	_HighestFallZ = _FallStartZ;
	_LastFallDistance = 0.0f;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Fall] START | Z: %.2f"),
		_FallStartZ
	);
}

void ASizeShiftCharacter::FinishFallTracking()
{
	const float landingZ = GetActorLocation().Z;

	_LastFallDistance =
		FMath::Max(
			0.0f,
			_HighestFallZ - landingZ
		);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[Fall] END | StartZ: %.2f | HighestZ: %.2f | LandingZ: %.2f | Distance: %.2f"
		),
		_FallStartZ,
		_HighestFallZ,
		landingZ,
		_LastFallDistance
	);

	// --------------------------------
	// Fall Damage
	// --------------------------------

	if (_LastFallDistance > _SafeFallDistance)
	{
		if (_HealthComponent)
		{
			_HealthComponent->ApplyDamage(
				_HealthComponent->GetMaxHealth()
			);
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"[Fall Damage] FATAL | Distance: %.2f"
			),
			_LastFallDistance
		);
	}

	_IsTrackingFall = false;
}

ASizeShiftBox* ASizeShiftCharacter::GetStandingBox() const
{
	return _StandingBox;
}

void ASizeShiftCharacter::SetMovementState(
	E_PlayerMovementState newState)
{
	_MovementState = newState;
}

E_PlayerMovementState ASizeShiftCharacter::GetMovementState() const
{
	return _MovementState;
}

// Called to bind functionality to input
void ASizeShiftCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* enhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (_MoveAction)
		{
			enhancedInputComponent->BindAction(
				_MoveAction,
				ETriggerEvent::Triggered,
				this,
				&ASizeShiftCharacter::Move
			);

			enhancedInputComponent->BindAction(
				_MoveAction,
				ETriggerEvent::Completed,
				this,
				&ASizeShiftCharacter::StopClimbingMovement
			);
		}

		if (_LookAction)
		{
			enhancedInputComponent->BindAction(
				_LookAction,
				ETriggerEvent::Triggered,
				this,
				&ASizeShiftCharacter::Look
			);
		}

		if (_JumpAction)
		{
			enhancedInputComponent->BindAction(
				_JumpAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::Jump
			);

			enhancedInputComponent->BindAction(
				_JumpAction,
				ETriggerEvent::Completed,
				this,
				&ACharacter::StopJumping
			);
		}
		
		if (_SprintAction)
		{
			enhancedInputComponent->BindAction(
				_SprintAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::StartSprint
			);

			enhancedInputComponent->BindAction(
				_SprintAction,
				ETriggerEvent::Completed,
				this,
				&ASizeShiftCharacter::StopSprint
			);
		}
		

		if (_CrouchAction)
		{
			enhancedInputComponent->BindAction(
				_CrouchAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::ToggleCrouch
			);
		}

		if (_SizeUpAction)
		{
			enhancedInputComponent->BindAction(
				_SizeUpAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::IncreaseSize
			);
		}

		if (_SizeDownAction)
		{
			enhancedInputComponent->BindAction(
				_SizeDownAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::DecreaseSize
			);
		}

		if (_InteractAction)
		{
			enhancedInputComponent->BindAction(
				_InteractAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::Interact
			);
		}

		if (_AdjustHoldDistanceAction)
		{
			enhancedInputComponent->BindAction(
				_AdjustHoldDistanceAction,
				ETriggerEvent::Triggered,
				this,
				&ASizeShiftCharacter::AdjustHoldDistance
			);
		}

		if (_HoldBoxAsideAction)
		{
			enhancedInputComponent->BindAction(
				_HoldBoxAsideAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::ToggleThrowAimMode
			);
		}
	}
}

//---------------------------------------Size Shift----------------------------------------------------------

void ASizeShiftCharacter::IncreaseSize()
{
	if (!_SizeShiftGun)
	{
		return;
	}

	if (_SizeShiftGun->IsHoldingPickupThrow())
	{
		_SizeShiftGun->TryThrow();
		return;
	}

	if (_SizeShiftGun->IsHoldingPushPull())
	{
		_SizeShiftGun->PushPullBurst();

		return;
	}

	_SizeShiftGun->IncreaseSize();
}

void ASizeShiftCharacter::DecreaseSize()
{
	if (!_SizeShiftGun)
	{
		return;
	}

	if (_SizeShiftGun->IsHoldingPickupThrow())
	{
		
		return;
	}

	_SizeShiftGun->DecreaseSize();
}

void ASizeShiftCharacter::Interact()
{
	if (!_SizeShiftGun)
	{
		return;
	}

	if (_MovementState == E_PlayerMovementState::Climbing)
	{
		return;
	}

	_SizeShiftGun->Interact();
}

//------------------------------------------------------------------

void ASizeShiftCharacter::AdjustHoldDistance(
	const FInputActionValue& value)
{
	if (_SizeShiftGun)
	{
		_SizeShiftGun->AdjustHoldDistance(
			value.Get<float>()
		);
	}
}

void ASizeShiftCharacter::ToggleThrowAimMode()
{
	if (_SizeShiftGun)
	{
		_SizeShiftGun->ToggleThrowAimMode();
	}
}

//--------------------------------------------------------------------

UHealthComponent* ASizeShiftCharacter::GetHealthComponent() const
{
	return _HealthComponent;
}

void ASizeShiftCharacter::HandleDeath()
{
	if (_UXComponent)
	{
		_UXComponent->PlayDeathSound();
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Character] HANDLE DEATH")
	);	

	Respawn();
}

void ASizeShiftCharacter::Respawn()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Respawn] RESPawning...")
	);

	FVector targetLocation = _RespawnLocation;
	FRotator targetRotation = _RespawnRotation;

	if (_CurrentCheckpoint)
	{
		targetLocation =
			_CurrentCheckpoint->GetRespawnLocation();

		targetRotation =
			_CurrentCheckpoint->GetRespawnRotation();

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Respawn] Using Checkpoint")
		);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Respawn] Using Initial Spawn")
		);
	}

	SetActorLocationAndRotation(
		targetLocation,
		targetRotation
	);

	GetCharacterMovement()->StopMovementImmediately();

	if (_HealthComponent)
	{
		_HealthComponent->ResetHealth();
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"[Respawn] Complete | Location: X=%.2f Y=%.2f Z=%.2f"
		),
		targetLocation.X,
		targetLocation.Y,
		targetLocation.Z
	);
}

void ASizeShiftCharacter::SetCurrentCheckpoint(
	ASizeShiftCheckpoint* checkpoint
)
{
	if (!checkpoint)
	{
		return;
	}

	_CurrentCheckpoint = checkpoint;

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[Respawn] Checkpoint Updated")
	);
}
