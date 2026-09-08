#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SizeShiftGun.generated.h"

class ASizeShiftBox;

UENUM(BlueprintType)
enum class ESizeShiftGunState : uint8
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
    float TargetTraceDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Interaction")
    float InteractionRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float HoldDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float ThrowSpeed = 1200.0f;

    void AdjustHoldDistance(float ScrollValue);

    void ToggleThrowAimMode();

    void TryThrow();

    bool IsHoldingPushPull() const;

    void PushPullBurst();
    

protected:

    // ============================================================
    // LIFECYCLE
    // ============================================================

    virtual void BeginPlay() override;

    // ============================================================
    // COMPONENTS
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun")
    UStaticMeshComponent* GunMesh;

    // ============================================================
    // TARGET
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Target")
    ASizeShiftBox* CurrentAimTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Interaction")
    ASizeShiftBox* ActiveInteractionTarget;

    // ============================================================
    // STATE
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|State")
    ESizeShiftGunState GunState;

    void SetGunState(ESizeShiftGunState NewState);

    void ClearInteraction();

    // ============================================================
    // PICKUP / THROW
    // ============================================================

    bool StartPickupHold();

    void StopPickupHold();

    void UpdatePickupThrow();

    void UpdateHeldBox();

    void SetHeldBoxPhysics(bool bIsHeld);

    FVector GetHoldLocation() const;

    // ============================================================
    // PUSH / PULL
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float PushPullDistance = 150.0f;

    bool StartPushPull();

    void UpdatePushPull(float DeltaTime);

    void StopPushPull();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float PushPullFollowSpeed = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float PushPullVelocityInterpSpeed = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float PushPullDeadZone = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float PushPullDirectionInterpSpeed = 5.0f;

    FVector SmoothedPushPullDirection =
        FVector::ZeroVector;

    bool bHasPushPullDirection = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float PushPullBurstImpulse = 500000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    FRotator PushPullFacingRotationOffset =
        FRotator::ZeroRotator;

    float CurrentPushPullDistance = 0.0f;

    // ==============================================================
    // Throw Aim
    // ==============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Visual")
    FVector ThrowPreviewOffset =
        FVector(100.0f, 80.0f, -70.0f);

    bool bIsThrowAimMode = false;

    float ThrowAimPreviousHoldDistance = 0.0f;

    void SetThrowAimMode(bool bNewThrowAimMode);

    void ShowHeldBoxPreview();

    void HideHeldBoxPreview();

    // ============================================================
    // INTERACTION HELPERS
    // ============================================================

    bool IsWithinInteractionRange() const;

    void StartInteraction();

    void StopInteraction();

    // ============================================================
    // DEBUG
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Debug")
    bool bEnableDebug = true;

    void PrintGunStatus(const FString& Message) const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float HoldDistanceStep = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float MinHoldDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float MaxHoldDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    float AutoDropDistance = 650.0f;

    float DefaultHoldDistance = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Visual")
    UStaticMeshComponent* HeldBoxPreview;


private:

    FString GetGunStateName() const;

    void UpdatePushPullFacing();

    FRotator HeldBoxRotation = FRotator::ZeroRotator;

    APlayerController* GetPlayerController() const;

};