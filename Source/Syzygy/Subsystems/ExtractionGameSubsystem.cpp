// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UExtractionGameSubsystem Implementation

#include "Subsystems/ExtractionGameSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// USubsystem Interface
// ─────────────────────────────────────────────────────────────────────────────

void UExtractionGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentPhase = ERunPhase::PreRun;

	UE_LOG(LogTemp, Log, TEXT("[ExtractionGameSubsystem] Initialized."));
}

void UExtractionGameSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExfilTimerHandle);
		World->GetTimerManager().ClearTimer(ExfilTickHandle);
	}

	Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Run Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void UExtractionGameSubsystem::StartRun(FName MapID)
{
	if (CurrentPhase != ERunPhase::PreRun && CurrentPhase != ERunPhase::PostRun)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[ExtractionGameSubsystem] StartRun called while phase is %s — ignored."),
			*UEnum::GetValueAsString(CurrentPhase));
		return;
	}

	CurrentMapID = MapID;
	NPCAlertedMap.Empty();
	ExfilTimeRemaining = RunDuration;

	SetPhase(ERunPhase::Infiltration);

	RunStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	// Start the main exfil countdown timer.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ExfilTimerHandle,
			this,
			&UExtractionGameSubsystem::OnExfilTimerExpired,
			RunDuration,
			false
		);

		// Tick timer for HUD updates.
		World->GetTimerManager().SetTimer(
			ExfilTickHandle,
			this,
			&UExtractionGameSubsystem::OnExfilTick,
			ExfilTickInterval,
			true  // Looping
		);
	}

	UE_LOG(LogTemp, Log, TEXT("[ExtractionGameSubsystem] Run started on map '%s'. Duration: %.0fs."),
		*MapID.ToString(), RunDuration);
}

void UExtractionGameSubsystem::TriggerExfilPhase()
{
	if (CurrentPhase == ERunPhase::Active)
	{
		SetPhase(ERunPhase::ExfilPhase);
		OnExfilPhaseStarted.Broadcast();

		UE_LOG(LogTemp, Log, TEXT("[ExtractionGameSubsystem] Exfil phase triggered. %.0fs remaining."),
			ExfilTimeRemaining);
	}
}

void UExtractionGameSubsystem::EndRun(bool bSuccess, float ExtractedValue)
{
	// Stop all timers.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExfilTimerHandle);
		World->GetTimerManager().ClearTimer(ExfilTickHandle);
	}

	// Calculate run duration.
	const float Now            = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	const float RunDurationSec = Now - RunStartTime;

	// Calculate payout.
	// Formula: ExtractedValue * SuccessPayoutMultiplier * (1 - alerts * AlertPenaltyPerNPC)
	// Penalty is clamped so payout never goes negative.
	float Payout = 0.f;
	if (bSuccess)
	{
		const int32 Alerts          = GetAlertCount();
		const float AlertPenalty    = FMath::Clamp(Alerts * AlertPenaltyPerNPC, 0.f, 0.9f);
		Payout = ExtractedValue * SuccessPayoutMultiplier * (1.f - AlertPenalty);
	}

	FRunResult Result;
	Result.bSuccess            = bSuccess;
	Result.TotalPayout         = Payout;
	Result.RunDurationSeconds  = RunDurationSec;
	Result.AlertCount          = GetAlertCount();

	SetPhase(ERunPhase::PostRun);
	OnRunEnded.Broadcast(Result);

	UE_LOG(LogTemp, Log,
		TEXT("[ExtractionGameSubsystem] Run ended. Success=%d | Payout=%.0f | Duration=%.1fs | Alerts=%d"),
		bSuccess, Payout, RunDurationSec, Result.AlertCount);
}

// ─────────────────────────────────────────────────────────────────────────────
// NPC Alert Tracking
// ─────────────────────────────────────────────────────────────────────────────

void UExtractionGameSubsystem::RegisterNPCAlert(FName NPCID)
{
	if (!NPCAlertedMap.Contains(NPCID))
	{
		NPCAlertedMap.Add(NPCID, true);
		UE_LOG(LogTemp, Log, TEXT("[ExtractionGameSubsystem] NPC '%s' alerted to player."), *NPCID.ToString());
	}
}

bool UExtractionGameSubsystem::HasNPCAlerted(FName NPCID) const
{
	const bool* Found = NPCAlertedMap.Find(NPCID);
	return Found && *Found;
}

int32 UExtractionGameSubsystem::GetAlertCount() const
{
	return NPCAlertedMap.Num();
}

// ─────────────────────────────────────────────────────────────────────────────
// Private
// ─────────────────────────────────────────────────────────────────────────────

void UExtractionGameSubsystem::SetPhase(ERunPhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return;
	}
	CurrentPhase = NewPhase;
	OnRunPhaseChanged.Broadcast(NewPhase);
}

void UExtractionGameSubsystem::OnExfilTimerExpired()
{
	UE_LOG(LogTemp, Warning, TEXT("[ExtractionGameSubsystem] Zone collapsed — time expired."));
	SetPhase(ERunPhase::Collapsed);
	ExfilTimeRemaining = 0.f;

	// Treat expiry as a failed run with whatever value the player is carrying.
	// The player character / game mode should call EndRun with extracted value.
	// We auto-end here to ensure the subsystem stays consistent.
	EndRun(false, 0.f);
}

void UExtractionGameSubsystem::OnExfilTick()
{
	if (ExfilTimeRemaining > 0.f)
	{
		ExfilTimeRemaining = FMath::Max(0.f, ExfilTimeRemaining - ExfilTickInterval);
		OnExfilTimerUpdated.Broadcast(ExfilTimeRemaining);

		// Trigger exfil warning when threshold is crossed.
		if (CurrentPhase == ERunPhase::Active
			&& ExfilTimeRemaining <= ExfilWarningThreshold)
		{
			TriggerExfilPhase();
		}
	}
}
