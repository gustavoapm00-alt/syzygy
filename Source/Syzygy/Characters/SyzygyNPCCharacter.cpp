// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — ASyzygyNPCCharacter Implementation

#include "Characters/SyzygyNPCCharacter.h"
#include "Subsystems/ExtractionGameSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

ASyzygyNPCCharacter::ASyzygyNPCCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// AIPerception — configure sight, hearing, and damage senses.
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius                = 1500.f;
	SightConfig->LoseSightRadius            = 1800.f;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies   = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals  = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 800.f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies  = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

	UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->ConfigureSense(*HearingConfig);
	AIPerception->ConfigureSense(*DamageConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyNPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Resolve archetype name unless manually overridden.
	if (NPCArchetypeName.IsEmpty() || NPCArchetypeName == TEXT("UNIT-7"))
	{
		NPCArchetypeName = GetArchetypeName(Archetype);
	}

	// Wire perception delegate.
	AIPerception->OnPerceptionUpdated.AddDynamic(this, &ASyzygyNPCCharacter::OnPerceptionUpdated);

	// NOTE: Inworld NPC component setup should occur in Blueprint BeginPlay
	// using the pattern from TSD Part 7.2:
	//   InworldComp->SetCharacterName(NPCArchetypeName)
	//   InworldComp->OnUtteranceDelegate.AddDynamic(this, &OnNPCSpeech_Implementation)
	// This keeps the C++ clean until the Inworld plugin is installed.

	// Run behavior tree via AIController.
	if (BehaviorTreeAsset)
	{
		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			AIC->RunBehaviorTree(BehaviorTreeAsset);
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Alert State
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyNPCCharacter::SetAlertState(ENPCAlertState NewState)
{
	if (AlertState == NewState)
	{
		return;
	}

	const ENPCAlertState OldState = AlertState;
	AlertState = NewState;
	OnAlertStateChanged.Broadcast(OldState, NewState);

	// Report alert to global subsystem for payout penalty calculation.
	if (NewState == ENPCAlertState::Alerted || NewState == ENPCAlertState::Combat)
	{
		ReportAlertToSubsystem();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Perception
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyNPCCharacter::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	// Iterate updated actors and check if any is the player.
	for (AActor* Actor : UpdatedActors)
	{
		if (!Actor) continue;

		FActorPerceptionBlueprintInfo Info;
		AIPerception->GetActorsPerception(Actor, Info);

		for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
		{
			if (Stimulus.WasSuccessfullySensed())
			{
				// Escalate alert level based on sense type.
				if (AlertState == ENPCAlertState::Passive)
				{
					SetAlertState(ENPCAlertState::Suspicious);
				}
				else if (AlertState == ENPCAlertState::Suspicious)
				{
					SetAlertState(ENPCAlertState::Alerted);
				}
				break;
			}
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Inworld Voice
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyNPCCharacter::OnNPCSpeech_Implementation(const FString& UtteranceText)
{
	// Base implementation logs the utterance.
	// Blueprint subclass overrides to trigger lip sync + audio playback.
	UE_LOG(LogTemp, Log, TEXT("[NPC %s] '%s'"), *NPCArchetypeName, *UtteranceText);
}

// ─────────────────────────────────────────────────────────────────────────────
// Private Helpers
// ─────────────────────────────────────────────────────────────────────────────

FString ASyzygyNPCCharacter::GetArchetypeName(ENPCArchetype Arch)
{
	switch (Arch)
	{
		case ENPCArchetype::Guard:     return TEXT("UNIT-7");
		case ENPCArchetype::Executive: return TEXT("NAKAMURA");
		case ENPCArchetype::Informant: return TEXT("ZERO");
		default:                       return TEXT("GENERIC_NPC");
	}
}

void ASyzygyNPCCharacter::ReportAlertToSubsystem()
{
	if (NPCID == NAME_None)
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UExtractionGameSubsystem* Sub = GI->GetSubsystem<UExtractionGameSubsystem>())
		{
			Sub->RegisterNPCAlert(NPCID);
		}
	}
}
