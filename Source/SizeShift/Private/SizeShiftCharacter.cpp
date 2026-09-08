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
#include "GameFramework/CharacterMovementComponent.h"
#include "Blueprint/UserWidget.h"

// Sets default values
ASizeShiftCharacter::ASizeShiftCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));

	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());

	FirstPersonCamera->SetRelativeLocation(
		FVector(0.0f, 0.0f, 64.0f)
	);

	FirstPersonCamera->bUsePawnControlRotation = true;

	SizeShiftGunComponent =
		CreateDefaultSubobject<UChildActorComponent>(
			TEXT("SizeShiftGun")
		);

	SizeShiftGunComponent->SetupAttachment(
		FirstPersonCamera
	);

}

void ASizeShiftCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SizeShiftGunComponent && SizeShiftGunClass)
	{
		SizeShiftGunComponent->SetChildActorClass(
			SizeShiftGunClass
		);
	}
}

// Called when the game starts or when spawned
void ASizeShiftCharacter::BeginPlay()
{
	Super::BeginPlay();

	OriginalCameraLocation = FirstPersonCamera->GetRelativeLocation();

	OriginalWalkSpeed =
		GetCharacterMovement()->MaxWalkSpeed;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bEnablePhysicsInteraction = false;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}
	}

	if (SizeShiftGunComponent)
	{
		SizeShiftGun =
			Cast<ASizeShiftGun>(
				SizeShiftGunComponent->GetChildActor()
			);
	}

	if (SizeShiftGun)
	{
		SizeShiftGun->SetOwner(this);
		SizeShiftGun->SetInstigator(this);
	}

	if (CrosshairWidgetClass)
	{
		CrosshairWidget =
			CreateWidget<UUserWidget>(
				GetWorld(),
				CrosshairWidgetClass
			);

		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport();
		}
	}
}

void ASizeShiftCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector =
		Value.Get<FVector2D>();

	if (MovementState == EPlayerMovementState::Climbing)
	{
		UpdateClimbingMovement(MovementVector.Y);
		return;
	}

	AddMovementInput(
		GetActorForwardVector(),
		MovementVector.Y
	);

	AddMovementInput(
		GetActorRightVector(),
		MovementVector.X
	);
}

void ASizeShiftCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ASizeShiftCharacter::Jump()
{
	if (MovementState == EPlayerMovementState::Climbing)
	{
		return;
	}

	ACharacter::Jump();
}

void ASizeShiftCharacter::ToggleCrouch()
{
	if (MovementState == EPlayerMovementState::Climbing)
	{
		return;
	}

	if (MovementState == EPlayerMovementState::Sprinting)
	{
		StopSprint();
	}

	if (MovementState == EPlayerMovementState::Crouching)
	{
		UnCrouch();

		FirstPersonCamera->SetRelativeLocation(
			OriginalCameraLocation
		);

		SetMovementState(EPlayerMovementState::Normal);
	}
	else
	{
		Crouch();

		FVector CameraLocation =
			FirstPersonCamera->GetRelativeLocation();

		CameraLocation.Z = 40.0f;

		FirstPersonCamera->SetRelativeLocation(
			CameraLocation
		);

		SetMovementState(EPlayerMovementState::Crouching);
	}
}

void ASizeShiftCharacter::StartSprint()
{
	if (MovementState == EPlayerMovementState::Crouching)
	{
		UnCrouch();
		FirstPersonCamera->SetRelativeLocation(OriginalCameraLocation);
	}

	if (MovementState == EPlayerMovementState::Climbing)
	{
		return;
	}

	SetMovementState(EPlayerMovementState::Sprinting);

	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ASizeShiftCharacter::StopSprint()
{
	if (MovementState != EPlayerMovementState::Sprinting)
	{
		return;
	}

	SetMovementState(EPlayerMovementState::Normal);

	GetCharacterMovement()->MaxWalkSpeed = OriginalWalkSpeed;
}

void ASizeShiftCharacter::StartClimbing(ALadder* Ladder)
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT(">>> START CLIMBING | Location: %s"),
		*GetActorLocation().ToString()
	);

	if (!Ladder)
	{
		return;
	}

	if (MovementState == EPlayerMovementState::Climbing)
	{
		return;
	}

	if (SizeShiftGun)
	{
		SizeShiftGun->CancelInteraction();
	}

	CurrentLadder = Ladder;

	const FTransform LadderTransform =
		Ladder->GetActorTransform();

	FVector LocalLocation =
		LadderTransform.InverseTransformPosition(
			GetActorLocation()
		);

	// Keep current height, only align horizontally.
	LocalLocation.X = -50.0f;
	LocalLocation.Y = 50.0f;

	const FVector AlignedLocation =
		LadderTransform.TransformPosition(
			LocalLocation
		);

	SetActorLocation(
		AlignedLocation,
		false
	);

	UCharacterMovementComponent* Movement =
		GetCharacterMovement();

	OriginalMovementMode = Movement->MovementMode;
	OriginalGravityScale = Movement->GravityScale;

	// Stop any existing movement
	Movement->Velocity = FVector::ZeroVector;

	// Cancel sprint / crouch
	if (MovementState == EPlayerMovementState::Sprinting)
	{
		Movement->MaxWalkSpeed = OriginalWalkSpeed;
	}

	if (MovementState == EPlayerMovementState::Crouching)
	{
		UnCrouch();

		FirstPersonCamera->SetRelativeLocation(
			OriginalCameraLocation
		);
	}

	SetMovementState(EPlayerMovementState::Climbing);

	Movement->GravityScale = 0.0f;
	Movement->SetMovementMode(MOVE_Flying);
}

void ASizeShiftCharacter::StopClimbing()
{
	UE_LOG(
		LogTemp,
		Warning,
		TEXT(">>> STOP CLIMBING | Location: %s"),
		*GetActorLocation().ToString()
	);

	if (MovementState != EPlayerMovementState::Climbing)
	{
		return;
	}

	UCharacterMovementComponent* Movement =
		GetCharacterMovement();

	Movement->Velocity = FVector::ZeroVector;

	Movement->GravityScale = OriginalGravityScale;
	Movement->SetMovementMode(OriginalMovementMode);

	CurrentLadder = nullptr;

	SetMovementState(EPlayerMovementState::Normal);
}

void ASizeShiftCharacter::StopClimbingMovement()
{
	if (MovementState != EPlayerMovementState::Climbing)
	{
		return;
	}

	FVector Velocity = GetCharacterMovement()->Velocity;

	Velocity.Z = 0.0f;

	GetCharacterMovement()->Velocity = Velocity;
}

void ASizeShiftCharacter::UpdateClimbingMovement(float InputZ)
{
	if (!CurrentLadder)
	{
		return;
	}

	const FTransform LadderTransform =
		CurrentLadder->GetActorTransform();

	const FVector LocalLocation =
		LadderTransform.InverseTransformPosition(
			GetActorLocation()
		);

	const float LadderHeight =
		CurrentLadder->GetLadderHeight();

	const float PlayerHalfHeight =
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const float MinZ = PlayerHalfHeight;

	const float BottomThreshold = 1.0f;
	const float TopThreshold = 1.0f;

	const float MaxZ = LadderHeight + PlayerHalfHeight;

	// Bottom
	if (LocalLocation.Z <= MinZ + BottomThreshold &&
		InputZ < 0.0f)
	{
		ExitLadderBottom();
		return;
	}

	// Top
	if (LocalLocation.Z >= MaxZ - TopThreshold &&
		InputZ > 0.0f)
	{
		ExitLadderTop();
		return;
	}

	if (FMath::IsNearlyZero(InputZ))
	{
		GetCharacterMovement()->Velocity.Z = 0.0f;
		return;
	}

	FVector Velocity =
		GetCharacterMovement()->Velocity;

	Velocity.X = 0.0f;
	Velocity.Y = 0.0f;
	Velocity.Z = InputZ * ClimbSpeed;

	GetCharacterMovement()->Velocity = Velocity;
}

void ASizeShiftCharacter::ExitLadderTop()
{
	if (!CurrentLadder)
	{
		return;
	}

	const FTransform LadderTransform =
		CurrentLadder->GetActorTransform();

	const float LadderHeight =
		CurrentLadder->GetLadderHeight();

	const float PlayerHalfHeight =
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// Move the player onto the top platform.
	const FVector LocalExitLocation(
		10.0f,
		50.0f,
		LadderHeight + PlayerHalfHeight
	);

	const FVector ExitLocation =
		LadderTransform.TransformPosition(
			LocalExitLocation
		);

	SetActorLocation(
		ExitLocation,
		false
	);

	StopClimbing();
}

void ASizeShiftCharacter::ExitLadderBottom()
{
	if (!CurrentLadder)
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

	const FTransform LadderTransform =
		CurrentLadder->GetActorTransform();

	const float PlayerHalfHeight =
		GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FVector LocalExitLocation(
		-150.0f,
		50.0f,
		PlayerHalfHeight
	);

	const FVector ExitLocation =
		LadderTransform.TransformPosition(
			LocalExitLocation
		);

	UE_LOG(
		LogTemp,
		Warning,
		TEXT("BOTTOM EXIT LOCATION: %s"),
		*ExitLocation.ToString()
	);

	SetActorLocation(ExitLocation, false);

	StopClimbing();
}

void ASizeShiftCharacter::SetMovementState(
	EPlayerMovementState NewState)
{
	MovementState = NewState;
}

EPlayerMovementState ASizeShiftCharacter::GetMovementState() const
{
	return MovementState;
}

// Called to bind functionality to input
void ASizeShiftCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(
				MoveAction,
				ETriggerEvent::Triggered,
				this,
				&ASizeShiftCharacter::Move
			);

			EnhancedInputComponent->BindAction(
				MoveAction,
				ETriggerEvent::Completed,
				this,
				&ASizeShiftCharacter::StopClimbingMovement
			);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(
				LookAction,
				ETriggerEvent::Triggered,
				this,
				&ASizeShiftCharacter::Look
			);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(
				JumpAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::Jump
			);

			EnhancedInputComponent->BindAction(
				JumpAction,
				ETriggerEvent::Completed,
				this,
				&ACharacter::StopJumping
			);
		}
		
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::StartSprint
			);

			EnhancedInputComponent->BindAction(
				SprintAction,
				ETriggerEvent::Completed,
				this,
				&ASizeShiftCharacter::StopSprint
			);
		}
		

		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(
				CrouchAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::ToggleCrouch
			);
		}

		if (SizeUpAction)
		{
			EnhancedInputComponent->BindAction(
				SizeUpAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::IncreaseSize
			);
		}

		if (SizeDownAction)
		{
			EnhancedInputComponent->BindAction(
				SizeDownAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::DecreaseSize
			);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(
				InteractAction,
				ETriggerEvent::Started,
				this,
				&ASizeShiftCharacter::Interact
			);
		}

		if (AdjustHoldDistanceAction)
		{
			EnhancedInputComponent->BindAction(
				AdjustHoldDistanceAction,
				ETriggerEvent::Triggered,
				this,
				&ASizeShiftCharacter::AdjustHoldDistance
			);
		}

		if (HoldBoxAsideAction)
		{
			EnhancedInputComponent->BindAction(
				HoldBoxAsideAction,
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
	if (!SizeShiftGun)
	{
		return;
	}

	if (SizeShiftGun->IsHoldingPickupThrow())
	{
		SizeShiftGun->TryThrow();
		return;
	}

	if (SizeShiftGun->IsHoldingPushPull())
	{
		SizeShiftGun->PushPullBurst();

		return;
	}

	SizeShiftGun->IncreaseSize();
}

void ASizeShiftCharacter::DecreaseSize()
{
	if (!SizeShiftGun)
	{
		return;
	}

	if (SizeShiftGun->IsHoldingPickupThrow())
	{
		
		return;
	}

	SizeShiftGun->DecreaseSize();
}

void ASizeShiftCharacter::Interact()
{
	if (!SizeShiftGun)
	{
		return;
	}

	if (MovementState == EPlayerMovementState::Climbing)
	{
		return;
	}

	SizeShiftGun->Interact();
}

//------------------------------------------------------------------

void ASizeShiftCharacter::AdjustHoldDistance(
	const FInputActionValue& Value)
{
	if (SizeShiftGun)
	{
		SizeShiftGun->AdjustHoldDistance(
			Value.Get<float>()
		);
	}
}

void ASizeShiftCharacter::ToggleThrowAimMode()
{
	if (SizeShiftGun)
	{
		SizeShiftGun->ToggleThrowAimMode();
	}
}
