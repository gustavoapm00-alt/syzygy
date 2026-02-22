// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — ASyzygyWeapon Implementation

#include "Weapons/SyzygyWeapon.h"
#include "Components/SkeletalMeshComponent.h"

ASyzygyWeapon::ASyzygyWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASyzygyWeapon::Fire_Implementation()
{
	// Base implementation: consume one round.
	// Subclasses override to add projectile/hitscan logic, muzzle flash, SFX etc.
	if (CurrentAmmoInMagazine > 0)
	{
		--CurrentAmmoInMagazine;
	}
}

void ASyzygyWeapon::Equip_Implementation(AActor* NewOwner)
{
	SetOwner(NewOwner);
	WeaponMesh->SetVisibility(true);
}

void ASyzygyWeapon::Unequip_Implementation()
{
	WeaponMesh->SetVisibility(false);
}
