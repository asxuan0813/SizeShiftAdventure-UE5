#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    // Initialize current health from the final MaxHealth value.
    _CurrentHealth = _MaxHealth;
}

void UHealthComponent::ApplyDamage(float damageAmount)
{
    if (damageAmount <= 0.0f)
    {
        return;
    }

    if (IsDead())
    {
        return;
    }

    _CurrentHealth = FMath::Clamp(
        _CurrentHealth - damageAmount,
        0.0f,
        _MaxHealth
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Health] Damage: %.2f | Health: %.2f / %.2f"),
        damageAmount,
        _CurrentHealth,
        _MaxHealth
    );

    if (_CurrentHealth <= 0.0f)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("[Health] DEATH")
        );

        _OnDeath.Broadcast();
    }
}

void UHealthComponent::Heal(float healAmount)
{
    if (healAmount <= 0.0f)
    {
        return;
    }

    if (IsDead())
    {
        return;
    }

    _CurrentHealth = FMath::Clamp(
        _CurrentHealth + healAmount,
        0.0f,
        _MaxHealth
    );

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Health] Heal: %.2f | Health: %.2f / %.2f"),
        healAmount,
        _CurrentHealth,
        _MaxHealth
    );
}

float UHealthComponent::GetCurrentHealth() const
{
    return _CurrentHealth;
}

float UHealthComponent::GetMaxHealth() const
{
    return _MaxHealth;
}

bool UHealthComponent::IsDead() const
{
    return _CurrentHealth <= 0.0f;
}

void UHealthComponent::ResetHealth()
{
    _CurrentHealth = _MaxHealth;

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("[Health] RESET | Health: %.2f / %.2f"),
        _CurrentHealth,
        _MaxHealth
    );
}