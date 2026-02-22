// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UHealthComponent
// CLASS 02 of 05 — Universal health system. Attached to both Player and NPCs.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Damage Type Enum
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical    UMETA(DisplayName = "Physical"),
	Energy      UMETA(DisplayName = "Energy"),
	Explosive   UMETA(DisplayName = "Explosive")
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnDamageTaken,
	float,         DamageAmount,
	EDamageType,   DamageType,
	AActor*,       InstigatorActor
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnHealed,
	float, HealAmount
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnDeath,
	AActor*, InstigatorActor
);

// ─────────────────────────────────────────────────────────────────────────────
// UHealthComponent
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Syzygy), meta = (BlueprintSpawnableComponent))
class SYZYGY_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	// ── Core API ──────────────────────────────────────────────────────────────

	/**
	 * Apply damage to this actor. Armor absorbs damage first, then health.
	 * Explosive damage bypasses 50% of armor.
	 * Returns the actual health damage dealt (after armor mitigation).
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Health")
	float ApplyDamage(float RawDamage, EDamageType DamageType, AActor* InstigatorActor);

	/**
	 * Restore health, clamped to MaxHealth. Does not restore armor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Health")
	void ApplyHealing(float Amount);

	// ── State Queries ─────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Syzygy|Health")
	bool IsAlive() const { return CurrentHealth > 0.f; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Health")
	bool IsInvulnerable() const { return bInvulnerable; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Syzygy|Health")
	float GetArmorPercent() const;

	// ── Setters (for GAS / subsystem-driven adjustments) ─────────────────────

	UFUNCTION(BlueprintCallable, Category = "Syzygy|Health")
	void SetInvulnerable(bool bNewValue) { bInvulnerable = bNewValue; }

	// ── Delegates ─────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Health")
	FOnDamageTaken OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Health")
	FOnHealed OnHealed;

	/** Fires once when CurrentHealth reaches 0. InstigatorActor is the killer. */
	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Health")
	FOnDeath OnDeath;

	// ── Properties ───────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Syzygy|Health")
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Syzygy|Health")
	float CurrentHealth = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Syzygy|Health")
	float MaxArmor = 50.f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Syzygy|Health")
	float CurrentArmor = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY()
	bool bInvulnerable = false;

	/** Prevents OnDeath from firing more than once. */
	bool bDeathBroadcast = false;

	/**
	 * Resolves damage after armor mitigation.
	 * Returns the health damage to actually apply.
	 */
	float ResolveDamage(float RawDamage, EDamageType DamageType);
};
