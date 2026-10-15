#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SizeShiftGun.generated.h"

class ASizeShiftBox;
class USizeShiftUXComponent;

// Describes whether the gun is idle or controlling an active interaction
UENUM(BlueprintType)
enum class E_SizeShiftGunState : uint8
{
    Idle,
    Interacting
};

// Finds boxes and coordinates size shifting, holding, throwing, and push/pull actions
UCLASS()
class SIZESHIFT_API ASizeShiftGun : public AActor
{
    GENERATED_BODY()

public:

    // -------------------------------------------------------------------------
    // Lifecycle, aiming, and player commands
    // -------------------------------------------------------------------------

    ASizeShiftGun();

    //Updates targeting and active interactions each frame
    virtual void Tick(float DeltaTime) override;

    // Returns or refreshes the box currently under the player's aim
    ASizeShiftBox* GetAimTarget() const;

    void UpdateAimTarget();

    // Changes the size of a valid aim target
    void IncreaseSize();

    void DecreaseSize();

    // Starts or cancels the applicable interaction for the current target
    void Interact();

    void CancelInteraction();

    // Pickup, throw, and push/pull commands called by the character.
    bool IsHoldingPickupThrow() const;

    void Throw();

    void Drop();

    void AdjustHoldDistance(float scrollValue);

    void ToggleThrowAimMode();

    void TryThrow();

    bool IsHoldingPushPull() const;

    void PushPullBurst();

    // -------------------------------------------------------------------------
    // Designer settings
    // -------------------------------------------------------------------------

    // Trace and range values used to find and validate targets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Target")
    float _TargetTraceDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Interaction")
    float _InteractionRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Size Shift")
    float _SizeShiftRange = 600.0f;

    // Default physics values for held and thrown boxes
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _HoldDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _ThrowSpeed = 1200.0f;

    // Height offset for the target interaction prompt
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|UX")
    float _PromptHeightOffset = 30.0f;

protected:

    // -------------------------------------------------------------------------
    // Lifecycle, components, and interaction state
    // -------------------------------------------------------------------------

    virtual void BeginPlay() override;

    // Gun visuals and feedback components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
    UStaticMeshComponent* _GunMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|UX")
    USizeShiftUXComponent* _UXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Visual")
    UStaticMeshComponent* _HeldBoxPreview;

    // Aim target, active target, and the state that controls their update flow.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Target")
    ASizeShiftBox* _CurrentAimTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Interaction")
    ASizeShiftBox* _ActiveInteractionTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|State")
    E_SizeShiftGunState _GunState;

    void SetGunState(E_SizeShiftGunState newState);

    void ClearInteraction();

    // Checks used before size shifting or starting an interaction
    bool HasAimTarget() const;

    bool IsWithinInteractionRange() const;

    bool IsWithinSizeShiftRange() const;

    bool CanInteractWithCurrentTarget() const;

    bool CanCurrentlySizeShift() const;

    void StartInteraction();

    void StopInteraction();

    // -------------------------------------------------------------------------
    // Pickup and throw implementation
    // -------------------------------------------------------------------------

    // Starts, updates, and ends pickup/throw control
    bool StartPickupHold();

    void StopPickupHold();

    void UpdatePickupThrow();

    void UpdateHeldBox();

    void SetHeldBoxPhysics(bool isHeld);

    FVector GetHoldLocation() const;

    // Throw-aim preview state and helpers
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Visual")
    FVector _ThrowPreviewOffset =
        FVector(100.0f, 80.0f, -70.0f);

    bool _IsThrowAimMode = false;

    float _ThrowAimPreviousHoldDistance = 0.0f;

    void SetThrowAimMode(bool newThrowAimMode);

    void ShowHeldBoxPreview();

    void HideHeldBoxPreview();

    // -------------------------------------------------------------------------
    // Push/pull implementation
    // -------------------------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullFollowSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullVelocityInterpSpeed = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullDeadZone = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullDirectionInterpSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullBurstImpulse = 500000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    FRotator _PushPullFacingRotationOffset =
        FRotator::ZeroRotator;

    // Runtime values that smooth the current push/pull operation
    FVector _SmoothedPushPullDirection =
        FVector::ZeroVector;

    bool _HasPushPullDirection = false;

    float _CurrentPushPullDistance = 0.0f;

    bool StartPushPull();

    void UpdatePushPull(float deltaTime);

    void StopPushPull();

    // -------------------------------------------------------------------------
    // Abilities, debug, and hold-distance configuration
    // -------------------------------------------------------------------------

    // Controls whether the player can use size-shift actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Abilities")
    bool _CanSizeShift = false;

    UFUNCTION(BlueprintCallable, Category = "Gun|Abilities")
    void UnlockSizeShift();

    UFUNCTION(BlueprintCallable, Category = "Gun|Abilities")
    void LockSizeShift();

    UFUNCTION(BlueprintPure, Category = "Gun|Abilities")
    bool CanSizeShift() const;

    // Enables diagnostic gun status messages
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Debug")
    bool _EnableDebug = true;

    void PrintGunStatus(const FString& message) const;

    // Limits and saved defaults used when adjusting held-box distance
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _HoldDistanceStep = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _MinHoldDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _MaxHoldDistance = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _AutoDropDistance = 500.0f;
    float _DefaultHoldDistance = 0.0f;

private:

    // -------------------------------------------------------------------------
    // Internal helpers
    // -------------------------------------------------------------------------

    // Formats state for diagnostics and refreshes presentation during updates
    FString GetGunStateName() const;

    void UpdatePushPullFacing();

    FRotator _HeldBoxRotation = FRotator::ZeroRotator;

    APlayerController* GetPlayerController() const;

    void RefreshUX();
};