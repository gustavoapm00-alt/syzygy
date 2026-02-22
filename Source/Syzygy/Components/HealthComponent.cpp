// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UHealthComponent Implementation

#include "Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	// Initialize to max values.
	CurrentHealth = MaxHealth;
	// Armor starts at 0 — must be granted via pickups.
}

// ─────────────────────────────────────────────────────────────────────────────
// Replication
// ─────────────────────────────────────────────────────────────────────────────

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, MaxHealth);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
	DOREPLIFETIME(UHealthComponent, MaxArmor);
	DOREPLIFETIME(UHealthComponent, CurrentArmor);
}

// ─────────────────────────────────────────────────────────────────────────────
// Core API
// ─────────────────────────────────────────────────────────────────────────────

float UHealthComponent::ApplyDamage(float RawDamage, EDamageType DamageType, AActor* InstigatorActor)
{
	// Zero or negative damage is a no-op.
	if (RawDamage <= 0.f || bInvulnerable || !IsAlive())
	{
		return 0.f;
	}

	const float HealthDamage = ResolveDamage(RawDamage, DamageType);

	CurrentHealth = FMath::Max(0.f, CurrentHealth - HealthDamage);
	OnDamageTaken.Broadcast(HealthDamage, DamageType, InstigatorActor);

	if (CurrentHealth <= 0.f && !bDeathBroadcast)
	{
		bDeathBroadcast = true;
		OnDeath.Broadcast(InstigatorActor);
	}

	return HealthDamage;
}

void UHealthComponent::ApplyHealing(float Amount)
{
	if (Amount <= 0.f || !IsAlive())
	{
		return;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);

	const float ActualHeal = CurrentHealth - OldHealth;
	if (ActualHeal > 0.f)
	{
		OnHealed.Broadcast(ActualHeal);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// State Queries
// ─────────────────────────────────────────────────────────────────────────────

float UHealthComponent::GetHealthPercent() const
{
	return (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;
}

float UHealthComponent::GetArmorPercent() const
{
	return (MaxArmor > 0.f) ? (CurrentArmor / MaxArmor) : 0.f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private — Damage Resolution
//
// Armor mitigation model:
//   Physical:  Armor absorbs damage 1:1 until depleted.
//   Energy:    Armor is 50% effective (energy punches through shielding).
//   Explosive: Armor is bypassed entirely; full damage goes to health.
// ─────────────────────────────────────────────────────────────────────────────

float UHealthComponent::ResolveDamage(float RawDamage, EDamageType DamageType)
{
	if (CurrentArmor <= 0.f || DamageType == EDamageType::Explosive)
	{
		// Explosive or no armor — full damage to health.
		return RawDamage;
	}

	float ArmorMitigationRatio = 1.f;

	switch (DamageType)
	{
		case EDamageType::Physical:
			ArmorMitigationRatio = 1.0f;  // Armor absorbs 100%
			break;
		case EDamageType::Energy:
			ArmorMitigationRatio = 0.5f;  // Armor absorbs 50%
			break;
		default:
			ArmorMitigationRatio = 1.0f;
			break;
	}

	const float MaxArmorAbsorb = CurrentArmor / ArmorMitigationRatio;
	const float ArmorAbsorbed  = FMath::Min(RawDamage, MaxArmorAbsorb);
	const float ArmorDepleted  = ArmorAbsorbed * ArmorMitigationRatio;

	CurrentArmor = FMath::Max(0.f, CurrentArmor - ArmorDepleted);

	// Remaining damage after armor absorption.
	return FMath::Max(0.f, RawDamage - ArmorAbsorbed);
}
