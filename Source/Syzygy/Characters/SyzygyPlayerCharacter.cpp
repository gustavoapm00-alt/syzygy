// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — ASyzygyPlayerCharacter Implementation

#include "Characters/SyzygyPlayerCharacter.h"
#include "Components/ExtractionInventoryComponent.h"
#include "Components/MovementStateComponent.h"
#include "Components/CombatComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

ASyzygyPlayerCharacter::ASyzygyPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Spring arm (camera boom).
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength         = 300.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag        = true;
	SpringArm->CameraLagSpeed          = 10.f;

	// Follow camera.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView             = 75.f;

	// Inventory.
	Inventory = CreateDefaultSubobject<UExtractionInventoryComponent>(TEXT("Inventory"));

	// Orient rotation to movement (third-person default).
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw   = false;
	bUseControllerRotationRoll  = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate              = FRotator(0.f, 540.f, 0.f);
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Register Enhanced Input mapping context.
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// Transition to Active phase when run starts (via subsystem listener in BP or here).
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick — Camera FOV
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCameraFOV(DeltaTime);
}

// ─────────────────────────────────────────────────────────────────────────────
// Input Binding
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOG(LogTemp, Error, TEXT("[PlayerCharacter] Enhanced Input Component not found! Ensure project uses Enhanced Input."));
		return;
	}

	if (IA_Move)       EIC->BindAction(IA_Move,        ETriggerEvent::Triggered, this, &ASyzygyPlayerCharacter::HandleMove);
	if (IA_Look)       EIC->BindAction(IA_Look,        ETriggerEvent::Triggered, this, &ASyzygyPlayerCharacter::HandleLook);
	if (IA_Sprint)     EIC->BindAction(IA_Sprint,      ETriggerEvent::Started,   this, &ASyzygyPlayerCharacter::HandleSprintStart);
	if (IA_Sprint)     EIC->BindAction(IA_Sprint,      ETriggerEvent::Completed, this, &ASyzygyPlayerCharacter::HandleSprintEnd);
	if (IA_Jump)       EIC->BindAction(IA_Jump,        ETriggerEvent::Started,   this, &ASyzygyPlayerCharacter::HandleJump);
	if (IA_Slide)      EIC->BindAction(IA_Slide,       ETriggerEvent::Started,   this, &ASyzygyPlayerCharacter::HandleSlideStart);
	if (IA_Slide)      EIC->BindAction(IA_Slide,       ETriggerEvent::Completed, this, &ASyzygyPlayerCharacter::HandleSlideEnd);
	if (IA_Fire)       EIC->BindAction(IA_Fire,        ETriggerEvent::Started,   this, &ASyzygyPlayerCharacter::HandleFireStart);
	if (IA_Fire)       EIC->BindAction(IA_Fire,        ETriggerEvent::Completed, this, &ASyzygyPlayerCharacter::HandleFireEnd);
	if (IA_Aim)        EIC->BindAction(IA_Aim,         ETriggerEvent::Started,   this, &ASyzygyPlayerCharacter::HandleAimStart);
	if (IA_Aim)        EIC->BindAction(IA_Aim,         ETriggerEvent::Completed, this, &ASyzygyPlayerCharacter::HandleAimEnd);
	if (IA_Reload)     EIC->BindAction(IA_Reload,      ETriggerEvent::Started,   this, &ASyzygyPlayerCharacter::HandleReload);
	if (IA_WeaponSlot1) EIC->BindAction(IA_WeaponSlot1, ETriggerEvent::Started,  this, &ASyzygyPlayerCharacter::HandleWeaponSlot1);
	if (IA_WeaponSlot2) EIC->BindAction(IA_WeaponSlot2, ETriggerEvent::Started,  this, &ASyzygyPlayerCharacter::HandleWeaponSlot2);
}

// ─────────────────────────────────────────────────────────────────────────────
// Enhanced Input Handlers
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyPlayerCharacter::HandleMove(const FInputActionValue& Value)
{
	const FVector2D MoveVec = Value.Get<FVector2D>();
	if (Controller && !MoveVec.IsNearlyZero())
	{
		const FRotator Rotation    = Controller->GetControlRotation();
		const FRotator YawRotation = FRotator(0.f, Rotation.Yaw, 0.f);

		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X), MoveVec.Y);
		AddMovementInput(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y), MoveVec.X);

		// Transition to Running if on the ground.
		if (MoveStateComp && GetCharacterMovement()->IsMovingOnGround())
		{
			const EMovementState State = MoveStateComp->GetCurrentState();
			if (State == EMovementState::Idle || State == EMovementState::Walking)
			{
				MoveStateComp->TransitionToState(EMovementState::Running);
			}
		}
	}
}

void ASyzygyPlayerCharacter::HandleLook(const FInputActionValue& Value)
{
	const FVector2D LookVec = Value.Get<FVector2D>();
	AddControllerYawInput(LookVec.X);
	AddControllerPitchInput(-LookVec.Y);
}

void ASyzygyPlayerCharacter::HandleSprintStart()
{
	if (MoveStateComp)
	{
		MoveStateComp->TransitionToState(EMovementState::Sprinting);
	}
}

void ASyzygyPlayerCharacter::HandleSprintEnd()
{
	if (MoveStateComp && MoveStateComp->GetCurrentState() == EMovementState::Sprinting)
	{
		MoveStateComp->TransitionToState(EMovementState::Running);
	}
}

void ASyzygyPlayerCharacter::HandleJump()
{
	Jump();
	if (MoveStateComp)
	{
		MoveStateComp->TransitionToState(EMovementState::Airborne);
	}
}

void ASyzygyPlayerCharacter::HandleSlideStart()
{
	if (MoveStateComp)
	{
		MoveStateComp->TransitionToState(EMovementState::Sliding);
	}
}

void ASyzygyPlayerCharacter::HandleSlideEnd()
{
	if (MoveStateComp && MoveStateComp->GetCurrentState() == EMovementState::Sliding)
	{
		MoveStateComp->TransitionToState(
			GetCharacterMovement()->IsMovingOnGround()
				? EMovementState::Running
				: EMovementState::Airborne
		);
	}
}

void ASyzygyPlayerCharacter::HandleFireStart()
{
	bFireHeld = true;
	if (CombatComp)
	{
		CombatComp->FireWeapon();

		// Start auto-fire timer for full-auto weapons.
		if (CombatComp->GetActiveWeapon())
		{
			const float FireRate   = CombatComp->GetActiveWeapon()->FireRate;
			const float FirePeriod = 60.f / FMath::Max(FireRate, 1.f);
			GetWorldTimerManager().SetTimer(AutoFireHandle, this, &ASyzygyPlayerCharacter::FireTick, FirePeriod, true);
		}
	}
}

void ASyzygyPlayerCharacter::HandleFireEnd()
{
	bFireHeld = false;
	GetWorldTimerManager().ClearTimer(AutoFireHandle);
}

void ASyzygyPlayerCharacter::HandleAimStart()
{
	if (CombatComp) CombatComp->AimDownSights(true);
}

void ASyzygyPlayerCharacter::HandleAimEnd()
{
	if (CombatComp) CombatComp->AimDownSights(false);
}

void ASyzygyPlayerCharacter::HandleReload()
{
	if (CombatComp) CombatComp->ReloadWeapon();
}

void ASyzygyPlayerCharacter::HandleWeaponSlot1()
{
	if (CombatComp) CombatComp->EquipWeapon(0);
}

void ASyzygyPlayerCharacter::HandleWeaponSlot2()
{
	if (CombatComp) CombatComp->EquipWeapon(1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Camera FOV Update
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyPlayerCharacter::UpdateCameraFOV(float DeltaTime)
{
	if (!FollowCamera || !MoveStateComp)
	{
		return;
	}

	float TargetFOV = MoveStateComp->GetTargetCameraFOV();

	// ADS overrides movement state FOV.
	if (CombatComp && CombatComp->IsAiming())
	{
		TargetFOV = AimFOV;
	}

	FollowCamera->FieldOfView = FMath::FInterpTo(
		FollowCamera->FieldOfView,
		TargetFOV,
		DeltaTime,
		FOVInterpSpeed
	);
}

// ─────────────────────────────────────────────────────────────────────────────
// Private — Auto-fire Tick
// ─────────────────────────────────────────────────────────────────────────────

void ASyzygyPlayerCharacter::FireTick()
{
	if (bFireHeld && CombatComp)
	{
		CombatComp->FireWeapon();
	}
}
