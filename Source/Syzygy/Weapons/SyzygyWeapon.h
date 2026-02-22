// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — ASyzygyWeapon
// Stub actor that UCombatComponent interacts with. Subclass in Blueprint per weapon type.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SyzygyWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
	SemiAuto    UMETA(DisplayName = "Semi-Auto"),
	FullAuto    UMETA(DisplayName = "Full-Auto"),
	BurstFire   UMETA(DisplayName = "Burst Fire")
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class SYZYGY_API ASyzygyWeapon : public AActor
{
	GENERATED_BODY()

public:
	ASyzygyWeapon();

	// ── Weapon Actions ────────────────────────────────────────────────────────

	/** Called by UCombatComponent when the trigger is pulled. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Syzygy|Weapon")
	void Fire();
	virtual void Fire_Implementation();

	/** Called when this weapon becomes the active weapon. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Syzygy|Weapon")
	void Equip(AActor* NewOwner);
	virtual void Equip_Implementation(AActor* NewOwner);

	/** Called when another weapon is equipped. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Syzygy|Weapon")
	void Unequip();
	virtual void Unequip_Implementation();

	// ── Weapon Properties ─────────────────────────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Weapon")
	FName WeaponID = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Weapon")
	FString DisplayName = TEXT("Unknown Weapon");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Weapon")
	EWeaponFireMode FireMode = EWeaponFireMode::SemiAuto;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Weapon")
	float Damage = 25.f;

	/** Rounds per minute. Full-auto burst timing derived from this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Weapon")
	float FireRate = 600.f;

	/** Duration in seconds to complete one reload cycle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Weapon")
	float ReloadTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Weapon")
	int32 MaxAmmoInMagazine = 30;

	UPROPERTY(BlueprintReadOnly, Category = "Syzygy|Weapon")
	int32 CurrentAmmoInMagazine = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Weapon")
	int32 MaxReserveAmmo = 90;

	UPROPERTY(BlueprintReadOnly, Category = "Syzygy|Weapon")
	int32 ReserveAmmo = 90;

	/** Camera shake class applied on each shot via UCombatComponent. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Weapon")
	TSubclassOf<UCameraShakeBase> FireCameraShake;

	/** Whether the magazine is currently empty. */
	UFUNCTION(BlueprintPure, Category = "Syzygy|Weapon")
	bool IsAmmoEmpty() const { return CurrentAmmoInMagazine <= 0; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Syzygy|Weapon")
	USkeletalMeshComponent* WeaponMesh;
};
