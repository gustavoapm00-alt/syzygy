// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UExtractionGameSubsystem
// CLASS 05 of 05 — The extraction loop's heartbeat. Replaces GameMode as run-state manager.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExtractionGameSubsystem.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Run Phase Enum
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ERunPhase : uint8
{
	PreRun       UMETA(DisplayName = "Pre-Run"),
	Infiltration UMETA(DisplayName = "Infiltration"),
	Active       UMETA(DisplayName = "Active"),
	ExfilPhase   UMETA(DisplayName = "Exfil Phase"),
	Collapsed    UMETA(DisplayName = "Collapsed"),
	PostRun      UMETA(DisplayName = "Post-Run")
};

// ─────────────────────────────────────────────────────────────────────────────
// FRunResult — Payload for OnRunEnded
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FRunResult
{
	GENERATED_BODY()

	/** True if player successfully extracted. False = died or zone collapsed. */
	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	/** Final payout in credits after risk modifiers. */
	UPROPERTY(BlueprintReadOnly)
	float TotalPayout = 0.f;

	/** Total seconds the run lasted (from StartRun to EndRun). */
	UPROPERTY(BlueprintReadOnly)
	float RunDurationSeconds = 0.f;

	/** Number of NPCs that detected the player. Higher = stealth penalty. */
	UPROPERTY(BlueprintReadOnly)
	int32 AlertCount = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunPhaseChanged, ERunPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExfilPhaseStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunEnded, FRunResult, RunResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExfilTimerUpdated, float, TimeRemaining);

// ─────────────────────────────────────────────────────────────────────────────
// UExtractionGameSubsystem
// ─────────────────────────────────────────────────────────────────────────────

UCLASS()
class SYZYGY_API UExtractionGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// ── USubsystem Interface ──────────────────────────────────────────────────

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── Run Lifecycle ─────────────────────────────────────────────────────────

	/**
	 * Begin a new extraction run on the specified map.
	 * Sets phase to Infiltration and starts the exfil countdown timer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Run")
	void StartRun(FName MapID);

	/**
	 * Called when the exfil timer threshold is reached.
	 * Transitions to ExfilPhase and broadcasts OnExfilPhaseStarted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Run")
	void TriggerExfilPhase();

	/**
	 * Conclude the current run. Calculates payout and fires OnRunEnded.
	 * @param bSuccess   True if player extracted successfully.
	 * @param ExtractedValue   Total market value of carried items at exfil.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Run")
	void EndRun(bool bSuccess, float ExtractedValue);

	// ── NPC Alert Tracking ────────────────────────────────────────────────────

	/** Mark an NPC (by ID) as having spotted the player this run. */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Run")
	void RegisterNPCAlert(FName NPCID);

	UFUNCTION(BlueprintPure, Category = "Syzygy|Run")
	bool HasNPCAlerted(FName NPCID) const;

	UFUNCTION(BlueprintPure, Category = "Syzygy|Run")
	int32 GetAlertCount() const;

	// ── Accessors ─────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Syzygy|Run")
	ERunPhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Run")
	float GetExfilTimeRemaining() const { return ExfilTimeRemaining; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Run")
	FName GetCurrentMapID() const { return CurrentMapID; }

	// ── Delegates ─────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Run")
	FOnRunPhaseChanged OnRunPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Run")
	FOnExfilPhaseStarted OnExfilPhaseStarted;

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Run")
	FOnRunEnded OnRunEnded;

	/** Fires every ExfilTickInterval seconds during the exfil countdown. */
	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Run")
	FOnExfilTimerUpdated OnExfilTimerUpdated;

	// ── Configuration ─────────────────────────────────────────────────────────

	/** Total run duration in seconds before zone collapses. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Run")
	float RunDuration = 600.f; // 10 minutes

	/** Seconds remaining when ExfilPhase is triggered. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Run")
	float ExfilWarningThreshold = 120.f; // 2 minutes warning

	/** How often OnExfilTimerUpdated fires (seconds). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Run")
	float ExfilTickInterval = 1.f;

	/** Payout multiplier applied to extracted value on success. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Run")
	float SuccessPayoutMultiplier = 1.0f;

	/** Penalty multiplier on payout for each NPC that alerted. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Run")
	float AlertPenaltyPerNPC = 0.05f;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Syzygy|Run", meta = (AllowPrivateAccess = "true"))
	ERunPhase CurrentPhase = ERunPhase::PreRun;

	float ExfilTimeRemaining = 0.f;

	FTimerHandle ExfilTimerHandle;
	FTimerHandle ExfilTickHandle;

	FName CurrentMapID = NAME_None;

	float RunStartTime = 0.f;

	/** NPC ID → true if they alerted to player this run. */
	UPROPERTY()
	TMap<FName, bool> NPCAlertedMap;

	void SetPhase(ERunPhase NewPhase);
	void OnExfilTimerExpired();
	void OnExfilTick();
};
