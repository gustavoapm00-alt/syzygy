// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UCombatComponent Implementation

#include "Components/CombatComponent.h"
#include "Weapons/SyzygyWeapon.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	WeaponInventory.SetNum(MaxWeaponSlots);
}

// ─────────────────────────────────────────────────────────────────────────────
// Combat Actions
// ─────────────────────────────────────────────────────────────────────────────

void UCombatComponent::FireWeapon()
{
	if (!CanFire())
	{
		return;
	}

	ActiveWeapon->Fire();
	OnWeaponFired.Broadcast();

	// Apply camera shake on the owning player controller (local only).
	if (ActiveWeapon->FireCameraShake)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientStartCameraShake(ActiveWeapon->FireCameraShake);
		}
	}
}

void UCombatComponent::AimDownSights(bool bAiming)
{
	if (bIsAiming == bAiming)
	{
		return;
	}

	bIsAiming = bAiming;
	OnAimStateChanged.Broadcast(bIsAiming);
}

void UCombatComponent::ReloadWeapon()
{
	if (bIsReloading || !ActiveWeapon)
	{
		return;
	}

	if (ActiveWeapon->ReserveAmmo <= 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[CombatComponent] No reserve ammo — cannot reload."));
		return;
	}

	if (ActiveWeapon->CurrentAmmoInMagazine == ActiveWeapon->MaxAmmoInMagazine)
	{
		return; // Already full.
	}

	bIsReloading = true;
	OnReloadStarted.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimerHandle,
		this,
		&UCombatComponent::OnReloadFinished,
		ActiveWeapon->ReloadTime,
		false
	);
}

void UCombatComponent::EquipWeapon(int32 SlotIndex)
{
	if (!WeaponInventory.IsValidIndex(SlotIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatComponent] EquipWeapon: Slot %d out of bounds."), SlotIndex);
		return;
	}

	ASyzygyWeapon* NewWeapon = WeaponInventory[SlotIndex];
	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CombatComponent] EquipWeapon: No weapon in slot %d."), SlotIndex);
		return;
	}

	if (NewWeapon == ActiveWeapon)
	{
		return; // Already equipped.
	}

	// Cancel reload if in progress.
	if (bIsReloading)
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
		bIsReloading = false;
	}

	// Unequip current.
	if (ActiveWeapon)
	{
		ActiveWeapon->Unequip();
	}

	// Equip new.
	ActiveWeapon    = NewWeapon;
	ActiveSlotIndex = SlotIndex;
	ActiveWeapon->Equip(GetOwner());

	OnWeaponEquipped.Broadcast(ActiveWeapon);
}

int32 UCombatComponent::AddWeapon(ASyzygyWeapon* Weapon)
{
	if (!Weapon)
	{
		return -1;
	}

	for (int32 i = 0; i < WeaponInventory.Num(); ++i)
	{
		if (!WeaponInventory[i])
		{
			WeaponInventory[i] = Weapon;
			// Auto-equip if no weapon is currently active.
			if (!ActiveWeapon)
			{
				EquipWeapon(i);
			}
			return i;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[CombatComponent] AddWeapon: Inventory full (%d slots)."), MaxWeaponSlots);
	return -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Queries
// ─────────────────────────────────────────────────────────────────────────────

bool UCombatComponent::CanFire() const
{
	return ActiveWeapon != nullptr
		&& !bIsReloading
		&& !ActiveWeapon->IsAmmoEmpty();
}

// ─────────────────────────────────────────────────────────────────────────────
// Private — Reload Completion
// ─────────────────────────────────────────────────────────────────────────────

void UCombatComponent::OnReloadFinished()
{
	if (!ActiveWeapon)
	{
		bIsReloading = false;
		return;
	}

	// Calculate how many rounds to move from reserve into magazine.
	const int32 RoundsNeeded    = ActiveWeapon->MaxAmmoInMagazine - ActiveWeapon->CurrentAmmoInMagazine;
	const int32 RoundsAvailable = FMath::Min(RoundsNeeded, ActiveWeapon->ReserveAmmo);

	ActiveWeapon->CurrentAmmoInMagazine += RoundsAvailable;
	ActiveWeapon->ReserveAmmo           -= RoundsAvailable;

	bIsReloading = false;
	OnReloadComplete.Broadcast();
}
