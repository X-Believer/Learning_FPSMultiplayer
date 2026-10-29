// Shuyang Xing All Rights Deserve


#include "Character/ShooterCharacter.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Combat/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/WeaponData.h"
#include "FPS/FPS.h"
#include "Game/ShooterGameModeBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Health/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/ShooterPlayerController.h"
#include "Weapon/Weapon.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetCharacterMovement()->MovementState.bCanCrouch = true;
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 0.0f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 15.0f;
	SpringArm->bUsePawnControlRotation = true;
	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(SpringArm);
	FirstPersonCamera->bUsePawnControlRotation = false;
	
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->SetupAttachment(FirstPersonCamera);
	Mesh1P->bOnlyOwnerSee = true;
	Mesh1P->bOwnerNoSee = false;
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	
	GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;
	GetMesh()->bReceivesDecals = false;
	
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetIsReplicated(true);
	
	DefaultFOV = 90.0f;
	TurningStatus = ETurnInPlace::NotTurning;
	bWeaponFirstReplicated = false;
	RespawnTime = 5.f;
}

bool AShooterCharacter::HasWeaponFirstReplicated() const
{
	return bWeaponFirstReplicated;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
	FirstPersonCamera->SetFieldOfView(DefaultFOV);
	StartingAimingRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
	if (AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetController()); IsValid(PC))
	{
		PC->bPawnAlive = true;
	}
}

bool AShooterCharacter::HasCurrentWeapon() const
{
	return CombatComponent && CombatComponent->GetCurrentWeapon() != nullptr;
}

void AShooterCharacter::BeginDestroy()
{
	Super::BeginDestroy();
	if (IsValid(CombatComponent))
	{
		CombatComponent->DestroyInventory();
	}
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CalculateFABRIKSocketTransform();
	CalculateTurnInPlaceParams(DeltaTime);
}

void AShooterCharacter::CalculateFABRIKSocketTransform()
{
	if (IsValid(CombatComponent) && IsValid(CombatComponent->GetCurrentWeapon()) && IsValid(CombatComponent->GetCurrentWeapon()->GetMesh3P()))
	{
		FABRIK_SocketTransform = CombatComponent->GetCurrentWeapon()->GetMesh3P()->GetSocketTransform("FABRIK_Socket", RTS_World);
		
		FVector OutLocation;
		FRotator OutRotation;
		GetMesh()->TransformToBoneSpace("hand_r", FABRIK_SocketTransform.GetLocation(), FABRIK_SocketTransform.GetRotation().Rotator(), OutLocation, OutRotation);
		
		FABRIK_SocketTransform.SetLocation(OutLocation);
		FABRIK_SocketTransform.SetRotation(OutRotation.Quaternion());
	}
}

void AShooterCharacter::CalculateTurnInPlaceParams(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if (Speed == 0.f && !bIsInAir)
	{
		FRotator CurrentAimRotation(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimingRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		
		if (TurningStatus == ETurnInPlace::NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		
		TurnInPlace(DeltaTime);
	}
	
	if (Speed > 0.f || bIsInAir)
	{
		StartingAimingRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		
		FRotator AimRotation = GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		
		TurningStatus = ETurnInPlace::NotTurning;
	}
	AO_Yaw *= -1.f;
}

void AShooterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningStatus = ETurnInPlace::Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningStatus = ETurnInPlace::Left;
	}
	if (TurningStatus != ETurnInPlace::NotTurning)
	{
		float InterpSpeed = 4.0f;
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, InterpSpeed);
		AO_Yaw = InterpAO_Yaw;
		
		if (FMath::Abs(AO_Yaw) < 5.f)
		{
			TurningStatus = ETurnInPlace::NotTurning;
			StartingAimingRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* ShooterInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	ShooterInputComponent->BindAction(CycleWeaponAction, ETriggerEvent::Started, this, &AShooterCharacter::Input_CycleWeapon);
	ShooterInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Started, this, &AShooterCharacter::Input_FireWeapon_Pressed);
	ShooterInputComponent->BindAction(FireWeaponAction, ETriggerEvent::Completed, this, &AShooterCharacter::Input_FireWeapon_Released);
	ShooterInputComponent->BindAction(ReloadWeaponAction, ETriggerEvent::Started, this, &AShooterCharacter::Input_ReloadWeapon);
	ShooterInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Started, this, &AShooterCharacter::Input_Aim_Pressed);
	ShooterInputComponent->BindAction(AimWeaponAction, ETriggerEvent::Completed, this, &AShooterCharacter::Input_Aim_Released);
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (IsValid(CombatComponent))
	{
		CombatComponent->SpawnInventory();
	}
}

void AShooterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (IsValid(CombatComponent))
	{
		CombatComponent->InitializeWeaponWidgets();
	}
}

FName AShooterCharacter::GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponType) const
{
	checkf(CombatComponent->WeaponData, TEXT("WeaponData is not set in CombatComponent"));
	return CombatComponent->WeaponData->GripPoints.FindChecked(WeaponType);
}

USkeletalMeshComponent* AShooterCharacter::GetMesh1P_Implementation() const
{
	return Mesh1P;
}

USkeletalMeshComponent* AShooterCharacter::GetMesh3P_Implementation() const
{
	return GetMesh();
}

void AShooterCharacter::WeaponReplicated_Implementation()
{
	if (!bWeaponFirstReplicated)
	{
		bWeaponFirstReplicated = true;
		OnWeaponFirstReplicated.Broadcast(CombatComponent->GetCurrentWeapon());
	}
}

AWeapon* AShooterCharacter::GetCurrentWeapon_Implementation() const
{
	return CombatComponent ? CombatComponent->GetCurrentWeapon() : nullptr;
}

int32 AShooterCharacter::GetReserveAmmo_Implementation() const
{
	return CombatComponent->CurrentReserveAmmo;
}

void AShooterCharacter::Notify_CycleWeapons_Implementation()
{
	CombatComponent->Notify_CycleWeapons();
}

void AShooterCharacter::Notify_ReloadWeapon_Implementation()
{
	CombatComponent->Notify_ReloadWeapon();
}

void AShooterCharacter::AddAmmo_Implementation(const FGameplayTag& WeaponType, int32 AmmoToAdd)
{
	if (HasAuthority() && IsValid(CombatComponent))
	{
		CombatComponent->AddAmmo(WeaponType, AmmoToAdd);
	}
}

bool AShooterCharacter::DoDamage_Implementation(float DamageAmount, AActor* DamageCauser)
{
	if (!IsValid(HealthComponent)) return false;
	
	HealthComponent->ChangeHealthByAmount(-DamageAmount, DamageCauser);
	const int32 MontageSelection = FMath::RandRange(0, HitReactions.Num() - 1);
	Multicast_HitReact(MontageSelection);
	
	return  false;
}

void AShooterCharacter::Multicast_HitReact_Implementation(int32 MontageIndex)
{
	if (GetNetMode() != NM_DedicatedServer && !IsLocallyControlled())
	{
		if (HitReactions.IsValidIndex(MontageIndex))
		{
			GetMesh()->GetAnimInstance()->Montage_Play(HitReactions[MontageIndex]);
		}
	}
}

FRotator AShooterCharacter::GetFixedAimRotation() const
{
	FRotator AimRotation = GetBaseAimRotation();
	if (AimRotation.Pitch > 90.0f && !IsLocallyControlled())
	{
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		AimRotation.Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimRotation.Pitch);
	}
	return AimRotation;
}

void AShooterCharacter::OnDeathStarted()
{
	if (HasAuthority())
	{
		CombatComponent->DestroyInventory();
		GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &ThisClass::DeathTimerFinished, RespawnTime);
	}
	if (GetNetMode() != NM_DedicatedServer)
	{
		DeathEffects();
		if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetController()))
		{
			DisableInput(PlayerController);
			if (PlayerController->IsLocalController())
			{
				if (AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(PlayerController))
				{
					ShooterPC->bPawnAlive = false;
				}
			}
		}
	}
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(FPSTraceChannels::ECC_Weapon, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(FPSTraceChannels::ECC_Weapon, ECR_Ignore);
}

void AShooterCharacter::DeathTimerFinished()
{
	AShooterGameModeBase* GM = Cast<AShooterGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (IsValid(GM))
	{
		GM->RequestRespawn(this, GetController());
	}
}

void AShooterCharacter::Input_CycleWeapon()
{
	CombatComponent->Initiate_CycleWeapon();
}

void AShooterCharacter::Input_FireWeapon_Pressed()
{
	CombatComponent->Initiate_FireWeapon_Pressed();
}

void AShooterCharacter::Input_FireWeapon_Released()
{
	CombatComponent->Initiate_FireWeapon_Released();
}

void AShooterCharacter::Input_ReloadWeapon()
{
	CombatComponent->Initiate_ReloadWeapon();
}

void AShooterCharacter::Input_Aim_Pressed()
{
	CombatComponent->Initiate_Aim_Pressed();
	OnAim(true);
}

void AShooterCharacter::Input_Aim_Released()
{
	CombatComponent->Initiate_Aim_Released();
	OnAim(false);
}

