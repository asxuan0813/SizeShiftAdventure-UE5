#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SizeShiftGun.generated.h"

class ASizeShiftBox;
class USizeShiftUXComponent;

UENUM(BlueprintType)
enum class E_SizeShiftGunState : uint8
{
    Idle,
    Interacting
};

UCLASS()
class SIZESHIFT_API ASizeShiftGun : public AActor
{
    GENERATED_BODY()

public:

    ASizeShiftGun();

    virtual void Tick(float DeltaTime) override;

    // ============================================================
    // AIM
    // ============================================================

    ASizeShiftBox* GetAimTarget() const;

    void UpdateAimTarget();

    // ============================================================
    // SIZE CONTROL
    // ============================================================

    void IncreaseSize();

    void DecreaseSize();

    // ============================================================
    // INTERACTION
    // ============================================================

    void Interact();

    void CancelInteraction();

    // ============================================================
    // PICKUP / THROW
    // ============================================================

    bool IsHoldingPickupThrow() const;

    void Throw();

    void Drop();

    // ============================================================
    // SETTINGS
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Target")
    float _TargetTraceDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Interaction")
    float _InteractionRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Size Shift")
    float _SizeShiftRange = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _HoldDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _ThrowSpeed = 1200.0f;

    void AdjustHoldDistance(float scrollValue);

    void ToggleThrowAimMode();

    void TryThrow();

    bool IsHoldingPushPull() const;

    void PushPullBurst();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|UX")
    float _PromptHeightOffset = 30.0f;

protected:

    // ============================================================
    // LIFECYCLE
    // ============================================================

    virtual void BeginPlay() override;

    // ============================================================
    // COMPONENTS
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
    UStaticMeshComponent* _GunMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|UX")
    USizeShiftUXComponent* _UXComponent;

    // ============================================================
    // TARGET
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Target")
    ASizeShiftBox* _CurrentAimTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Interaction")
    ASizeShiftBox* _ActiveInteractionTarget;

    // ============================================================
    // STATE
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|State")
    E_SizeShiftGunState _GunState;

    void SetGunState(E_SizeShiftGunState newState);

    void ClearInteraction();

    // ============================================================
    // PICKUP / THROW
    // ============================================================

    bool StartPickupHold();

    void StopPickupHold();

    void UpdatePickupThrow();

    void UpdateHeldBox();

    void SetHeldBoxPhysics(bool isHeld);

    FVector GetHoldLocation() const;

    // ============================================================
    // PUSH / PULL
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullDistance = 150.0f;

    bool StartPushPull();

    void UpdatePushPull(float deltaTime);

    void StopPushPull();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullFollowSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullVelocityInterpSpeed = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullDeadZone = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullDirectionInterpSpeed = 5.0f;

    FVector _SmoothedPushPullDirection =
        FVector::ZeroVector;

    bool _HasPushPullDirection = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float _PushPullBurstImpulse = 500000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    FRotator _PushPullFacingRotationOffset =
        FRotator::ZeroRotator;

    float _CurrentPushPullDistance = 0.0f;

    // ==============================================================
    // Throw Aim
    // ==============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Visual")
    FVector _ThrowPreviewOffset =
        FVector(100.0f, 80.0f, -70.0f);

    bool _IsThrowAimMode = false;

    float _ThrowAimPreviousHoldDistance = 0.0f;

    void SetThrowAimMode(bool newThrowAimMode);

    void ShowHeldBoxPreview();

    void HideHeldBoxPreview();

    // ============================================================
     // INTERACTION HELPERS
    // ============================================================

    bool HasAimTarget() const;

    bool IsWithinInteractionRange() const;

    bool IsWithinSizeShiftRange() const;

    bool CanInteractWithCurrentTarget() const;

    bool CanCurrentlySizeShift() const;

    void StartInteraction();
    void StopInteraction();

    // ============================================================
    // ABILITIES
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Abilities")
    bool _CanSizeShift = false;

    UFUNCTION(BlueprintCallable, Category = "Gun|Abilities")
    void UnlockSizeShift();

    UFUNCTION(BlueprintCallable, Category = "Gun|Abilities")
    void LockSizeShift();

    UFUNCTION(BlueprintPure, Category = "Gun|Abilities")
    bool CanSizeShift() const;

    // ============================================================
    // DEBUG
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Debug")
    bool _EnableDebug = true;

    void PrintGunStatus(const FString& message) const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _HoldDistanceStep = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _MinHoldDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _MaxHoldDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float _AutoDropDistance = 650.0f;

    float _DefaultHoldDistance = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Visual")
    UStaticMeshComponent* _HeldBoxPreview;


private:

    FString GetGunStateName() const;

    void UpdatePushPullFacing();

    FRotator _HeldBoxRotation = FRotator::ZeroRotator;

    APlayerController* GetPlayerController() const;

    void RefreshUX();
};