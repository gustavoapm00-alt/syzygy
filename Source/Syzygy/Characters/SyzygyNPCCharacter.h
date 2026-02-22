// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — ASyzygyNPCCharacter
// AI-controlled NPC. Integrates Behavior Tree, AIPerception, and Inworld AI voice.

#pragma once

#include "CoreMinimal.h"
#include "Characters/SyzygyCharacterBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "SyzygyNPCCharacter.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;

// ─────────────────────────────────────────────────────────────────────────────
// NPC Archetype — maps to Inworld AI personality models
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPCArchetype : uint8
{
	Guard       UMETA(DisplayName = "Guard (UNIT-7)"),
	Executive   UMETA(DisplayName = "Executive (NAKAMURA)"),
	Informant   UMETA(DisplayName = "Informant (ZERO)"),
	Generic     UMETA(DisplayName = "Generic")
};

// ─────────────────────────────────────────────────────────────────────────────
// NPC Alert State — mirrors Behavior Tree state
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ENPCAlertState : uint8
{
	Passive     UMETA(DisplayName = "Passive"),   // Patrol / idle
	Suspicious  UMETA(DisplayName = "Suspicious"), // Heard something
	Alerted     UMETA(DisplayName = "Alerted"),   // Full alert
	Combat      UMETA(DisplayName = "Combat"),    // Actively engaging
	Dialogue    UMETA(DisplayName = "Dialogue")   // Speaking with player
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnNPCAlertStateChanged,
	ENPCAlertState, OldState,
	ENPCAlertState, NewState
);

// ─────────────────────────────────────────────────────────────────────────────
// ASyzygyNPCCharacter
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class SYZYGY_API ASyzygyNPCCharacter : public ASyzygyCharacterBase
{
	GENERATED_BODY()

public:
	ASyzygyNPCCharacter();

	// ── Alert State API ───────────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Syzygy|NPC")
	void SetAlertState(ENPCAlertState NewState);

	UFUNCTION(BlueprintPure, Category = "Syzygy|NPC")
	ENPCAlertState GetAlertState() const { return AlertState; }

	// ── Inworld Voice (stub — wired via Blueprint when plugin present) ─────────

	/**
	 * Called by the Inworld NPC component's OnUtteranceDelegate.
	 * Routes to lip sync + audio playback. Override in Blueprint.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Syzygy|NPC|Voice")
	void OnNPCSpeech(const FString& UtteranceText);
	virtual void OnNPCSpeech_Implementation(const FString& UtteranceText);

	// ── Perception Events ─────────────────────────────────────────────────────

	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	// ── Delegates ─────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|NPC")
	FOnNPCAlertStateChanged OnAlertStateChanged;

	// ── Properties ───────────────────────────────────────────────────────────

	/** Which Inworld personality model to load. Maps to NPCArchetypeName below. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Syzygy|NPC")
	ENPCArchetype Archetype = ENPCArchetype::Guard;

	/**
	 * Inworld character name passed to InworldNPCComponent::SetCharacterName().
	 * Populated from Archetype in BeginPlay unless overridden.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Syzygy|NPC")
	FString NPCArchetypeName = TEXT("UNIT-7");

	/** Unique ID used by UExtractionGameSubsystem::RegisterNPCAlert(). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Syzygy|NPC")
	FName NPCID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Syzygy|NPC")
	UAIPerceptionComponent* AIPerception;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|NPC|AI")
	UBehaviorTree* BehaviorTreeAsset;

private:
	ENPCAlertState AlertState = ENPCAlertState::Passive;

	/** Maps archetype enum to Inworld character name. */
	static FString GetArchetypeName(ENPCArchetype Arch);

	/** Notify UExtractionGameSubsystem when NPC reaches Alerted/Combat state. */
	void ReportAlertToSubsystem();
};
