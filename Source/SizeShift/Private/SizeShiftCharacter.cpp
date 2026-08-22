// Fill out your copyright notice in the Description page of Project Settings.


#include "SizeShiftCharacter.h"
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

	SizeShiftGun = CreateDefaultSubobject<ASizeShiftGun>(
		TEXT("SizeShiftGun")
	);
}

// Called every frame
void ASizeShiftCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASizeShiftCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), MovementVector.Y);
	AddMovementInput(GetActorRightVector(), MovementVector.X);
}

void ASizeShiftCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ASizeShiftCharacter::StartCrouch()
{
	if (!bIsCrouched)
	{
		Crouch();
	}
}

void ASizeShiftCharacter::StopCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void ASizeShiftCharacter::ToggleCrouch()
{
	if (bIsCrouched)
	{
		StopCrouch();
	}
	else
	{
		StartCrouch();
	}
}

// Called when the game starts or when spawned
void ASizeShiftCharacter::BeginPlay()
{
	Super::BeginPlay();

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

	if (SizeShiftGunClass)
	{
		FActorSpawnParameters SpawnParams;

		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		SizeShiftGun =
			GetWorld()->SpawnActor<ASizeShiftGun>(
				SizeShiftGunClass,
				GetActorTransform(),
				SpawnParams
			);

		if (SizeShiftGun)
		{
			SizeShiftGun->AttachToComponent(
				FirstPersonCamera,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale
			);

			UE_LOG(
				LogTemp,
				Log,
				TEXT("[Character] SizeShiftGun spawned successfully")
			);
		}
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

			UE_LOG(
				LogTemp,
				Log,
				TEXT("[Character] Crosshair created successfully")
			);
		}
	}
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
				&ACharacter::Jump
			);

			EnhancedInputComponent->BindAction(
				JumpAction,
				ETriggerEvent::Completed,
				this,
				&ACharacter::StopJumping
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
		SizeShiftGun->Throw();
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
		SizeShiftGun->Drop();
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

	SizeShiftGun->Interact();
}


