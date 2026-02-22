// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — ASyzygyCharacterBase Implementation

#include "Characters/SyzygyCharacterBase.h"
#include "Components/HealthComponent.h"
#include "Components/CombatComponent.h"
#include "Components/MovementStateComponent.h"
#include "AbilitySystemComponent.h"

ASyzygyCharacterBase::ASyzygyCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	HealthComp    = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
	CombatComp    = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	MoveStateComp = CreateDefaultSubobject<UMovementStateComponent>(TEXT("MoveStateComp"));
	AbilityComp   = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilityComp"));
}

void ASyzygyCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Wire OnDeath delegate from HealthComponent.
	if (HealthComp)
	{
		HealthComp->OnDeath.AddDynamic(this, &ASyzygyCharacterBase::OnDeath_Implementation);
	}
}

UAbilitySystemComponent* ASyzygyCharacterBase::GetAbilitySystemComponent() const
{
	return AbilityComp;
}

void ASyzygyCharacterBase::OnDeath_Implementation(AActor* InstigatorActor)
{
	// Base implementation: disable collision, apply ragdoll physics.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}
