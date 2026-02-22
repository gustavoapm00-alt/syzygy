// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UMovementStateComponent
// CLASS 01 of 05 — The state machine governing all character locomotion.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MovementStateComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Movement State Enum
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle        UMETA(DisplayName = "Idle"),
	Walking     UMETA(DisplayName = "Walking"),
	Running     UMETA(DisplayName = "Running"),
	Sprinting   UMETA(DisplayName = "Sprinting"),
	Airborne    UMETA(DisplayName = "Airborne"),
	Sliding     UMETA(DisplayName = "Sliding"),
	Vaulting    UMETA(DisplayName = "Vaulting")
};

// ─────────────────────────────────────────────────────────────────────────────
// Movement Parameters Struct
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FMovementStateParams
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxSpeed = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Acceleration = 2048.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BrakingFriction = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TargetCameraFOV = 75.f;
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnMovementStateChanged,
	EMovementState, OldState,
	EMovementState, NewState
);

// ─────────────────────────────────────────────────────────────────────────────
// UMovementStateComponent
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Syzygy), meta = (BlueprintSpawnableComponent))
class SYZYGY_API UMovementStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMovementStateComponent();

	// ── State Machine ─────────────────────────────────────────────────────────

	/** Attempt a transition to NewState. Validates legal transition rules. */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Movement")
	bool TransitionToState(EMovementState NewState);

	/** Returns true if transitioning from CurrentState to Target is legal. */
	UFUNCTION(BlueprintPure, Category = "Syzygy|Movement")
	bool CanTransitionTo(EMovementState Target) const;

	/** Pushes MaxWalkSpeed and MaxAcceleration to the owning character's UCharacterMovementComponent. */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Movement")
	void UpdateMovementParams();

	// ── Accessors ─────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Syzygy|Movement")
	EMovementState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Movement")
	float GetTargetCameraFOV() const;

	// ── Delegates ─────────────────────────────────────────────────────────────

	/** Fires whenever the movement state changes successfully. */
	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Movement")
	FOnMovementStateChanged OnStateChanged;

protected:
	virtual void BeginPlay() override;

private:
	// ── State ─────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Syzygy|Movement", meta = (AllowPrivateAccess = "true"))
	EMovementState CurrentState = EMovementState::Idle;

	// ── Per-state Parameters (EditDefaultsOnly so designers tune in BP subclass) ──

	UPROPERTY(EditDefaultsOnly, Category = "Syzygy|Movement|Params")
	FMovementStateParams IdleParams      = { 0.f,    2048.f, 8.0f, 75.f };

	UPROPERTY(EditDefaultsOnly, Category = "Syzygy|Movement|Params")
	FMovementStateParams WalkingParams   = { 300.f,  2048.f, 8.0f, 75.f };

	UPROPERTY(EditDefaultsOnly, Category = "Syzygy|Movement|Params")
	FMovementStateParams RunningParams   = { 600.f,  2048.f, 8.0f, 78.f };

	UPROPERTY(EditDefaultsOnly, Category = "Syzygy|Movement|Params")
	FMovementStateParams SprintingParams = { 900.f,  2048.f, 6.0f, 88.f };

	UPROPERTY(EditDefaultsOnly, Category = "Syzygy|Movement|Params")
	FMovementStateParams AirborneParams  = { 600.f,  512.f,  0.2f, 82.f };

	UPROPERTY(EditDefaultsOnly, Category = "Syzygy|Movement|Params")
	FMovementStateParams SlidingParams   = { 1100.f, 0.f,    0.5f, 92.f };

	UPROPERTY(EditDefaultsOnly, Category = "Syzygy|Movement|Params")
	FMovementStateParams VaultingParams  = { 600.f,  2048.f, 4.0f, 80.f };

	// ── Helpers ───────────────────────────────────────────────────────────────

	const FMovementStateParams& GetParamsForState(EMovementState State) const;
	class UCharacterMovementComponent* GetCharacterMovement() const;
};
