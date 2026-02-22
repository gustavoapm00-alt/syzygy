// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UMovementStateComponent Implementation

#include "Components/MovementStateComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// ─────────────────────────────────────────────────────────────────────────────
// Legal Transition Table
//
// Key insight: Airborne → Sliding is explicitly illegal (TSD spec).
// Sliding → Airborne is allowed (ran off a ledge while sliding).
// Vaulting is entered externally (e.g., from Running/Walking via trace detection).
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
	// Returns true if OldState → NewState is a legal transition.
	bool IsLegalTransition(EMovementState Old, EMovementState New)
	{
		// Transitioning to the same state is always a no-op (handled above).
		// Format: illegal pairs are listed; everything else is allowed.
		struct FIllegalPair { EMovementState From; EMovementState To; };

		static constexpr FIllegalPair IllegalPairs[] =
		{
			// Cannot slide mid-air (must land first)
			{ EMovementState::Airborne,  EMovementState::Sliding  },
			// Cannot vault mid-air
			{ EMovementState::Airborne,  EMovementState::Vaulting },
			// Cannot sprint while sliding or vaulting
			{ EMovementState::Sliding,   EMovementState::Sprinting },
			{ EMovementState::Vaulting,  EMovementState::Sprinting },
		};

		for (const FIllegalPair& Pair : IllegalPairs)
		{
			if (Pair.From == Old && Pair.To == New)
			{
				return false;
			}
		}
		return true;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

UMovementStateComponent::UMovementStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void UMovementStateComponent::BeginPlay()
{
	Super::BeginPlay();
	// Apply initial parameters on first frame.
	UpdateMovementParams();
}

// ─────────────────────────────────────────────────────────────────────────────
// State Machine
// ─────────────────────────────────────────────────────────────────────────────

bool UMovementStateComponent::TransitionToState(EMovementState NewState)
{
	if (CurrentState == NewState)
	{
		return false;
	}

	if (!CanTransitionTo(NewState))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[MovementStateComponent] Illegal transition blocked: %s → %s"),
			*UEnum::GetValueAsString(CurrentState),
			*UEnum::GetValueAsString(NewState));
		return false;
	}

	const EMovementState OldState = CurrentState;
	CurrentState = NewState;

	UpdateMovementParams();
	OnStateChanged.Broadcast(OldState, NewState);

	return true;
}

bool UMovementStateComponent::CanTransitionTo(EMovementState Target) const
{
	return IsLegalTransition(CurrentState, Target);
}

// ─────────────────────────────────────────────────────────────────────────────
// Movement Parameters
// ─────────────────────────────────────────────────────────────────────────────

void UMovementStateComponent::UpdateMovementParams()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	const FMovementStateParams& Params = GetParamsForState(CurrentState);

	MoveComp->MaxWalkSpeed         = Params.MaxSpeed;
	MoveComp->MaxAcceleration      = Params.Acceleration;
	MoveComp->BrakingFrictionFactor = Params.BrakingFriction;

	// Sliding uses momentum — disable braking deceleration entirely.
	if (CurrentState == EMovementState::Sliding)
	{
		MoveComp->BrakingDecelerationWalking = 0.f;
	}
	else
	{
		MoveComp->BrakingDecelerationWalking = 2048.f;
	}
}

float UMovementStateComponent::GetTargetCameraFOV() const
{
	return GetParamsForState(CurrentState).TargetCameraFOV;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private Helpers
// ─────────────────────────────────────────────────────────────────────────────

const FMovementStateParams& UMovementStateComponent::GetParamsForState(EMovementState State) const
{
	switch (State)
	{
		case EMovementState::Walking:   return WalkingParams;
		case EMovementState::Running:   return RunningParams;
		case EMovementState::Sprinting: return SprintingParams;
		case EMovementState::Airborne:  return AirborneParams;
		case EMovementState::Sliding:   return SlidingParams;
		case EMovementState::Vaulting:  return VaultingParams;
		default:                        return IdleParams;
	}
}

UCharacterMovementComponent* UMovementStateComponent::GetCharacterMovement() const
{
	const ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	return OwnerChar ? OwnerChar->GetCharacterMovement() : nullptr;
}
