#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gun|Physics")
    UPhysicsHandleComponent* PhysicsHandle;

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

    // ============================================================
    // PICKUP / THROW
    // ============================================================

    void StartPhysicsHold();

    void StopPhysicsHold();

    FVector GetHoldLocation() const;

    // ============================================================
    // PUSH / PULL
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Push Pull")
    float PushPullDistance = 150.0f;

    FVector PushPullAnchorOffset = FVector::ZeroVector;

    void StartPushPull();

    void UpdatePushPull(float DeltaTime);

    void StopPushPull();

    FVector GetPushPullAnchorLocation() const;

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

    // ============================================================
    // INTERACTION HELPERS
    // ============================================================

    bool IsWithinInteractionRange() const;

    void StartInteraction();

    void StopInteraction();

    // ============================================================
    // STATE
    // ============================================================

    void SetGunState(ESizeShiftGunState NewState);

    // ============================================================
    // DEBUG
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun|Debug")
    bool bEnableDebug = true;

    void PrintGunStatus(const FString& Message) const;

};