// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UCombatComponent
// CLASS 04 of 05 — Weapon management, fire logic, ADS, reload. Fully decoupled.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ASyzygyWeapon;

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAimStateChanged, bool, bIsAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, ASyzygyWeapon*, NewWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponFired);

// ─────────────────────────────────────────────────────────────────────────────
// UCombatComponent
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Syzygy), meta = (BlueprintSpawnableComponent))
class SYZYGY_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	// ── Combat Actions ────────────────────────────────────────────────────────

	/**
	 * Fire the active weapon. Applies camera shake and fires OnWeaponFired delegate.
	 * Blocked during reload or when no weapon is equipped.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Combat")
	void FireWeapon();

	/**
	 * Toggle aim-down-sights. Triggers FOV lerp on the camera.
	 * Fires OnAimStateChanged delegate.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Combat")
	void AimDownSights(bool bAiming);

	/**
	 * Begin reload sequence. Locks FireWeapon for the duration of ActiveWeapon->ReloadTime.
	 * Ammo math is applied on completion.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Combat")
	void ReloadWeapon();

	/**
	 * Equip weapon at SlotIndex from WeaponInventory.
	 * Calls Unequip on current weapon and Equip on the new one.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Combat")
	void EquipWeapon(int32 SlotIndex);

	/**
	 * Directly add a weapon actor to the inventory (up to MaxWeaponSlots).
	 * Returns the slot index, or -1 if inventory is full.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Combat")
	int32 AddWeapon(ASyzygyWeapon* Weapon);

	// ── Queries ───────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Syzygy|Combat")
	bool IsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Combat")
	bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Combat")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Syzygy|Combat")
	ASyzygyWeapon* GetActiveWeapon() const { return ActiveWeapon; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Combat")
	int32 GetActiveSlotIndex() const { return ActiveSlotIndex; }

	// ── Delegates ─────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Combat")
	FOnAimStateChanged OnAimStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Combat")
	FOnWeaponEquipped OnWeaponEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Combat")
	FOnReloadStarted OnReloadStarted;

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Combat")
	FOnReloadComplete OnReloadComplete;

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Combat")
	FOnWeaponFired OnWeaponFired;

	// ── Configuration ─────────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Combat")
	int32 MaxWeaponSlots = 2;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<ASyzygyWeapon*> WeaponInventory;

	UPROPERTY()
	ASyzygyWeapon* ActiveWeapon = nullptr;

	int32 ActiveSlotIndex = -1;

	bool bIsAiming    = false;
	bool bIsReloading = false;

	FTimerHandle ReloadTimerHandle;

	void OnReloadFinished();
};
