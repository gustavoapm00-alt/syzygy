// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — ASyzygyPlayerCharacter
// Human-controlled player. Third-person with dynamic FOV and voice integration.

#pragma once

#include "CoreMinimal.h"
#include "Characters/SyzygyCharacterBase.h"
#include "InputActionValue.h"
#include "SyzygyPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UExtractionInventoryComponent;
class UInputMappingContext;
class UInputAction;

UCLASS(BlueprintType, Blueprintable)
class SYZYGY_API ASyzygyPlayerCharacter : public ASyzygyCharacterBase
{
	GENERATED_BODY()

public:
	ASyzygyPlayerCharacter();

	// ── Input Binding ─────────────────────────────────────────────────────────

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// ── Accessors ─────────────────────────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "Syzygy|Player")
	UExtractionInventoryComponent* GetInventory() const { return Inventory; }

	UFUNCTION(BlueprintPure, Category = "Syzygy|Player")
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ── Enhanced Input Handlers ───────────────────────────────────────────────

	UFUNCTION()
	void HandleMove(const FInputActionValue& Value);

	UFUNCTION()
	void HandleLook(const FInputActionValue& Value);

	UFUNCTION()
	void HandleSprintStart();

	UFUNCTION()
	void HandleSprintEnd();

	UFUNCTION()
	void HandleJump();

	UFUNCTION()
	void HandleSlideStart();

	UFUNCTION()
	void HandleSlideEnd();

	UFUNCTION()
	void HandleFireStart();

	UFUNCTION()
	void HandleFireEnd();

	UFUNCTION()
	void HandleAimStart();

	UFUNCTION()
	void HandleAimEnd();

	UFUNCTION()
	void HandleReload();

	UFUNCTION()
	void HandleWeaponSlot1();

	UFUNCTION()
	void HandleWeaponSlot2();

	// ── Camera FOV ────────────────────────────────────────────────────────────

	/** Smoothly interpolates camera FOV toward the target FOV for the current movement state. */
	void UpdateCameraFOV(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Camera")
	float FOVInterpSpeed = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Camera")
	float AimFOV = 60.f;

	// ── Components ────────────────────────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Syzygy|Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Syzygy|Components")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Syzygy|Components")
	UExtractionInventoryComponent* Inventory;

	// ── Input Assets (assign in BP subclass) ──────────────────────────────────

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_Move;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_Sprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_Jump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_Slide;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_Fire;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_Aim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_Reload;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_WeaponSlot1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Syzygy|Input")
	UInputAction* IA_WeaponSlot2;

private:
	/** True while fire input is held (for full-auto support). */
	bool bFireHeld = false;

	FTimerHandle AutoFireHandle;

	void FireTick();
};
