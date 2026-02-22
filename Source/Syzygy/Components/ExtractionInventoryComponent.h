// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — UExtractionInventoryComponent
// CLASS 03 of 05 — The extraction loop core mechanic. Carry weight = risk profile.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExtractionInventoryComponent.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// FExtractionItem — The fundamental unit of extracted value
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FExtractionItem
{
	GENERATED_BODY()

	/** Unique identifier used for loot tables and save data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemID = NAME_None;

	/** Human-readable name shown in the HUD. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString DisplayName = TEXT("Unknown Item");

	/** Contributes to carry weight. Overloading penalizes movement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float Weight = 1.f;

	/** Higher encryption levels fetch more value but take longer to crack at exfil. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 EncryptionLevel = 0;

	/** Base market value in credits. Modified by difficulty/economy at run end. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float MarketValue = 0.f;
};

// ─────────────────────────────────────────────────────────────────────────────
// Delegates
// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverloadStateChanged, bool, bIsOverloaded);

// ─────────────────────────────────────────────────────────────────────────────
// UExtractionInventoryComponent
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Syzygy), meta = (BlueprintSpawnableComponent))
class SYZYGY_API UExtractionInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExtractionInventoryComponent();

	// ── Core Inventory API ────────────────────────────────────────────────────

	/**
	 * Attempt to add an item to the inventory.
	 * Returns false if adding the item would exceed MaxCarryWeight.
	 * Fires OnInventoryChanged on success.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Inventory")
	bool TryAddItem(const FExtractionItem& Item);

	/**
	 * Remove the item at Index and spawn a BP_DroppedItem actor at character location.
	 * Does nothing if Index is out of bounds.
	 */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Inventory")
	void DropItem(int32 Index);

	/** Remove all items without spawning drop actors. Used on death / zone collapse. */
	UFUNCTION(BlueprintCallable, Category = "Syzygy|Inventory")
	void ClearInventory();

	// ── Queries ───────────────────────────────────────────────────────────────

	/** Sum of all item weights currently carried. */
	UFUNCTION(BlueprintPure, Category = "Syzygy|Inventory")
	float GetCurrentWeight() const;

	/** Sum of all item MarketValue fields. Displayed in HUD as run value. */
	UFUNCTION(BlueprintPure, Category = "Syzygy|Inventory")
	float CalculateRunValue() const;

	/** True when GetCurrentWeight() > MaxCarryWeight. */
	UFUNCTION(BlueprintPure, Category = "Syzygy|Inventory")
	bool IsOverloaded() const { return GetCurrentWeight() > MaxCarryWeight; }

	/** Returns the overload ratio (0–1+). Values above 1.0 = overloaded. */
	UFUNCTION(BlueprintPure, Category = "Syzygy|Inventory")
	float GetWeightRatio() const;

	UFUNCTION(BlueprintPure, Category = "Syzygy|Inventory")
	int32 GetItemCount() const { return Items.Num(); }

	// ── Delegates ─────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Inventory")
	FOnInventoryChanged OnInventoryChanged;

	/** Fires when transitioning between overloaded / not overloaded states. */
	UPROPERTY(BlueprintAssignable, Category = "Syzygy|Inventory")
	FOnOverloadStateChanged OnOverloadStateChanged;

	// ── Properties ───────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category = "Syzygy|Inventory")
	TArray<FExtractionItem> Items;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Inventory")
	float MaxCarryWeight = 25.f;

	/** Blueprint class used for the dropped item actor. Assign in BP subclass. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Inventory")
	TSubclassOf<AActor> DroppedItemClass;

private:
	/** Cached overload state to detect transitions. */
	bool bWasOverloaded = false;

	void CheckOverloadTransition();
};
