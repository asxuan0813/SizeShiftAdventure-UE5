#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize current health from the final MaxHealth value.
    CurrentHealth = MaxHealth;
}

void UHealthComponent::ApplyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f)
    {
        return;
    }

    if (IsDead())
    {
        return;
    }

    CurrentHealth = FMath::Clamp(
        CurrentHealth - DamageAmount,
        0.0f,
        MaxHealth
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Health] Damage: %.2f | Health: %.2f / %.2f"),
        DamageAmount,
        CurrentHealth,
        MaxHealth
    );

    if (CurrentHealth <= 0.0f)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Health] DEATH")
        );

        OnDeath.Broadcast();
    }
}

void UHealthComponent::Heal(float HealAmount)
{
    if (HealAmount <= 0.0f)
    {
        return;
    }

    if (IsDead())
    {
        return;
    }

    CurrentHealth = FMath::Clamp(
        CurrentHealth + HealAmount,
        0.0f,
        MaxHealth
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Health] Heal: %.2f | Health: %.2f / %.2f"),
        HealAmount,
        CurrentHealth,
        MaxHealth
    );
}

float UHealthComponent::GetCurrentHealth() const
{
    return CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
    return MaxHealth;
}

bool UHealthComponent::IsDead() const
{
    return CurrentHealth <= 0.0f;
}

void UHealthComponent::ResetHealth()
{
    CurrentHealth = MaxHealth;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Health] RESET | Health: %.2f / %.2f"),
        CurrentHealth,
        MaxHealth
    );
}