// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UExtractionInventoryComponent Implementation

#include "Components/ExtractionInventoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

UExtractionInventoryComponent::UExtractionInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Core Inventory API
// ─────────────────────────────────────────────────────────────────────────────

bool UExtractionInventoryComponent::TryAddItem(const FExtractionItem& Item)
{
	// Validate item.
	if (Item.ItemID == NAME_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Inventory] Attempted to add item with no ItemID — rejected."));
		return false;
	}

	// Weight gate.
	if (GetCurrentWeight() + Item.Weight > MaxCarryWeight)
	{
		UE_LOG(LogTemp, Log,
			TEXT("[Inventory] Cannot add '%s' — weight limit exceeded (%.1f / %.1f)."),
			*Item.DisplayName, GetCurrentWeight(), MaxCarryWeight);
		return false;
	}

	Items.Add(Item);
	OnInventoryChanged.Broadcast();
	CheckOverloadTransition();

	return true;
}

void UExtractionInventoryComponent::DropItem(int32 Index)
{
	if (!Items.IsValidIndex(Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Inventory] DropItem: Index %d out of bounds (count: %d)."), Index, Items.Num());
		return;
	}

	// Spawn a dropped item actor at owner location if a class is assigned.
	if (DroppedItemClass && GetOwner())
	{
		const FVector SpawnLocation  = GetOwner()->GetActorLocation();
		const FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Instigator = Cast<APawn>(GetOwner());

		GetWorld()->SpawnActor<AActor>(DroppedItemClass, SpawnLocation, SpawnRotation, SpawnParams);
	}

	Items.RemoveAt(Index);
	OnInventoryChanged.Broadcast();
	CheckOverloadTransition();
}

void UExtractionInventoryComponent::ClearInventory()
{
	Items.Empty();
	OnInventoryChanged.Broadcast();
	CheckOverloadTransition();
}

// ─────────────────────────────────────────────────────────────────────────────
// Queries
// ─────────────────────────────────────────────────────────────────────────────

float UExtractionInventoryComponent::GetCurrentWeight() const
{
	float Total = 0.f;
	for (const FExtractionItem& Item : Items)
	{
		Total += Item.Weight;
	}
	return Total;
}

float UExtractionInventoryComponent::CalculateRunValue() const
{
	float Total = 0.f;
	for (const FExtractionItem& Item : Items)
	{
		Total += Item.MarketValue;
	}
	return Total;
}

float UExtractionInventoryComponent::GetWeightRatio() const
{
	return (MaxCarryWeight > 0.f) ? (GetCurrentWeight() / MaxCarryWeight) : 0.f;
}

// ─────────────────────────────────────────────────────────────────────────────
// Private
// ─────────────────────────────────────────────────────────────────────────────

void UExtractionInventoryComponent::CheckOverloadTransition()
{
	const bool bNowOverloaded = IsOverloaded();
	if (bNowOverloaded != bWasOverloaded)
	{
		bWasOverloaded = bNowOverloaded;
		OnOverloadStateChanged.Broadcast(bNowOverloaded);
	}
}
