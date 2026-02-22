// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — ASyzygyCharacterBase
// Abstract base for ALL humanoid actors. Never instantiate directly.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "SyzygyCharacterBase.generated.h"

class UHealthComponent;
class UCombatComponent;
class UMovementStateComponent;
class UAbilitySystemComponent;
class UAttributeSet;

UCLASS(Abstract)
class SYZYGY_API ASyzygyCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASyzygyCharacterBase();

	// ── IAbilitySystemInterface ───────────────────────────────────────────────

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ── Accessors ─────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Syzygy|Character")
	UHealthComponent* GetHealthComponent() const { return HealthComp; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Character")
	UCombatComponent* GetCombatComponent() const { return CombatComp; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Character")
	UMovementStateComponent* GetMovementStateComponent() const { return MoveStateComp; }

protected:
	virtual void BeginPlay() override;

	/** Called by HealthComponent's OnDeath delegate. Override in subclasses. */
	UFUNCTION(BlueprintNativeEvent, Category = "Syzygy|Character")
	void OnDeath(AActor* InstigatorActor);
	virtual void OnDeath_Implementation(AActor* InstigatorActor);

	// ── Core Components ───────────────────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Syzygy|Components")
	UHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Syzygy|Components")
	UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Syzygy|Components")
	UMovementStateComponent* MoveStateComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Syzygy|Components")
	UAbilitySystemComponent* AbilityComp;
};
