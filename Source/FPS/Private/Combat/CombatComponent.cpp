// Shuyang Xing All Rights Deserve


#include "Combat/CombatComponent.h"

#include "Data/WeaponData.h"
#include "FPS/FPS.h"
#include "Interfaces/PlayerInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/Weapon.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TraceLength = 20000.0f;
	bAiming = false;
	bTriggerPressed = false;
	Local_WeaponIndex = 0;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwningPawn) || !OwningPawn->IsLocallyControlled()) return;
	
	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!IsValid(PC)) return;
	
	FVector EyesWorldLocation;
	FRotator EyesWorldRotation;
	PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
	const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
	
	const FVector StartLocation = EyesWorldLocation;
	const FVector EndLocation = EyesWorldLocation + (EyesWorldDirection * TraceLength);
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECollisionChannel::ECC_Pawn, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECollisionChannel::ECC_PhysicsBody, ECR_Block);
	
	GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, FPSTraceChannels::ECC_Weapon, QueryParams, ResponseParams);
	
	bHitPlayer = IsValid(HitResult.GetActor()) && HitResult.GetActor()->Implements<UPlayerInterface>();
	
	if (bHitPlayer != bHitPlayerLastFrame)
	{
		OnTargetingPlayerStatusChanged.Broadcast(bHitPlayer);
	}
	bHitPlayerLastFrame = bHitPlayer;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCombatComponent, Inventory);
	DOREPLIFETIME(UCombatComponent, CurrentWeapon);
	DOREPLIFETIME_CONDITION(UCombatComponent, bAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UCombatComponent, CurrentReserveAmmo, COND_OwnerOnly);
}

void UCombatComponent::Initiate_CycleWeapon()
{
	if (!IsValid(CurrentWeapon)) return;
	if (CurrentWeapon->WeaponStatus == EWeaponStatus::Cycling) return;
	
	AdvanceWeaponIndex();
	Local_CycleWeapon(Local_WeaponIndex);
}

int32 UCombatComponent::AdvanceWeaponIndex()
{
	if (Inventory.Num() >=2)
	{
		Local_WeaponIndex = (Local_WeaponIndex + 1) % Inventory.Num();
	}
	return Local_WeaponIndex;
}

void UCombatComponent::Local_CycleWeapon(int32 WeaponIndex)
{
	AWeapon* NextWeapon = Inventory[WeaponIndex];
	if (!IsValid(NextWeapon) || !IsValid(WeaponData)) return;
	CurrentWeapon->WeaponStatus = EWeaponStatus::Cycling;
	NextWeapon->WeaponStatus = EWeaponStatus::Cycling;
	
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	const bool bIsLocallyControlled = IsValid(OwningPawn) && OwningPawn->IsLocallyControlled();
	
	const FMontageData& MontageData = bIsLocallyControlled ? WeaponData->FirstPersonMontages.FindChecked(NextWeapon->WeaponType) : WeaponData->ThirdPersonMontages.FindChecked(NextWeapon->WeaponType);
	USkeletalMeshComponent* Mesh = bIsLocallyControlled ? IPlayerInterface::Execute_GetMesh1P(GetOwner()) : IPlayerInterface::Execute_GetMesh3P(GetOwner());
	if (IsValid(Mesh) && IsValid(MontageData.EquipMontage))
	{
		Mesh->GetAnimInstance()->Montage_Play(MontageData.EquipMontage);
	}
	if (bIsLocallyControlled)
	{
		Server_CycleWeapon(WeaponIndex);
		Mesh->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(this, &ThisClass::BlendOut_CycleWeapon);
	}
}

void UCombatComponent::Server_CycleWeapon_Implementation(int32 WeaponIndex)
{
	Local_WeaponIndex = WeaponIndex;
	Multicast_CycleWeapon(WeaponIndex);
}

void UCombatComponent::Multicast_CycleWeapon_Implementation(int32 WeaponIndex)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());

	if (!IsValid(OwningPawn)) return;
	if (!OwningPawn->IsLocallyControlled())
	{
		Local_WeaponIndex = WeaponIndex;
		Local_CycleWeapon(WeaponIndex);
	}
}

void UCombatComponent::Notify_CycleWeapons()
{
	if (!IsValid(CurrentWeapon)) return;
	
	AWeapon* NewWeapon = Inventory[Local_WeaponIndex];
	if (IsValid(NewWeapon))
	{
		EquipWeapon(NewWeapon);
	}
}

void UCombatComponent::Notify_ReloadWeapon()
{
	if (!IsValid(CurrentWeapon)) return;
	if (GetNetMode() == NM_ListenServer || GetNetMode() == NM_DedicatedServer || GetNetMode() == NM_Standalone)
	{
		const int32 EmptySpace = CurrentWeapon->MagCapacity - CurrentWeapon->Ammo;
		const int32 AmountToRefill = FMath::Min(EmptySpace, CurrentReserveAmmo);
		CurrentWeapon->Ammo += AmountToRefill;
		ReserveAmmoMap[CurrentWeapon->WeaponType] = FMath::Max(0, CurrentReserveAmmo - AmountToRefill);
		CurrentReserveAmmo = ReserveAmmoMap[CurrentWeapon->WeaponType];
		Client_ReloadWeapon(CurrentWeapon->Ammo, CurrentReserveAmmo);
	}
	CurrentWeapon->WeaponStatus = EWeaponStatus::Idle;
	if (bTriggerPressed && CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();
	}
}

void UCombatComponent::AddAmmo(const FGameplayTag& WeaponType, int32 AmmoToAdd)
{
	if (GetOwner()->HasAuthority() && IsValid(CurrentWeapon))
	{
		if (!ReserveAmmoMap.Contains(WeaponType))
		{
			ReserveAmmoMap.Add(WeaponType, 0);
		}
		const int32 NewAmmo = ReserveAmmoMap.FindChecked(WeaponType) + AmmoToAdd;
		ReserveAmmoMap[WeaponType] = FMath::Max(0, NewAmmo);
		
		if (CurrentWeapon->WeaponType.MatchesTagExact(WeaponType))
		{
			CurrentReserveAmmo = ReserveAmmoMap.FindChecked(WeaponType);
			if (CurrentWeapon->Ammo == 0 && NewAmmo > 0)
			{
				Server_ReloadWeapon();
			}
			
			OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterDynMatInst(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
			OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
		}
	}
}

void UCombatComponent::BlendOut_CycleWeapon(UAnimMontage* Montage, bool bInterrupted)
{
	UAnimInstance* AnimInstance = IPlayerInterface::Execute_GetMesh1P(GetOwner())->GetAnimInstance();
	
	if (IsValid(AnimInstance) && AnimInstance->OnMontageBlendingOut.IsAlreadyBound(this, &ThisClass::BlendOut_CycleWeapon))
	{
		AnimInstance->OnMontageBlendingOut.RemoveDynamic(this, &ThisClass::BlendOut_CycleWeapon);
	}
	
	CurrentWeapon->WeaponStatus = EWeaponStatus::Idle;
	OnReticleChanged.Broadcast(CurrentWeapon->GetReticleDynMatInst(), CurrentWeapon->ReticleParams, bHitPlayer);
	OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterDynMatInst(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
	OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	
	if (bTriggerPressed && CurrentWeapon->FireType == EFireType::Auto && CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();
	}
}

void UCombatComponent::Initiate_FireWeapon_Pressed()
{
	if (!IsValid(CurrentWeapon)) return;
	bTriggerPressed = true;
	
	if (CurrentWeapon->WeaponStatus == EWeaponStatus::Idle && CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();
	}
}

void UCombatComponent::Local_FireWeapon()
{
	if (!IsValid(CurrentWeapon)) return;
	ensure(IsValid(WeaponData));
	
	CurrentWeapon->WeaponStatus = EWeaponStatus::Firing;
	
	UAnimMontage* Montage1P = WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->WeaponType).FireMontage;
	USkeletalMeshComponent* Mesh1P = IPlayerInterface::Execute_GetMesh1P(GetOwner());
	if (IsValid(Mesh1P) && IsValid(Montage1P))
	{
		Mesh1P->GetAnimInstance()->Montage_Play(Montage1P);
	}
	
	FHitResult HitResult;
	CurrentWeapon->WeaponTrace(HitResult, TraceLength);
	
	EPhysicalSurface ImpactSurfaceType = HitResult.PhysMaterial.IsValid(false) ? HitResult.PhysMaterial->SurfaceType.GetValue() : SurfaceType1;
	CurrentWeapon->LocalFire(HitResult.ImpactPoint, HitResult.ImpactNormal, ImpactSurfaceType, true);
	
	OnRoundFired.Broadcast(CurrentWeapon->Ammo, CurrentWeapon->MagCapacity, CurrentReserveAmmo);
	
	GetWorld()->GetTimerManager().SetTimer(FireTimer, this, &ThisClass::FireTimerFinished, CurrentWeapon->FireTime, false);
	
	Server_FireWeapon(HitResult);
}

void UCombatComponent::OnRep_CurrentReserveAmmo()
{
	if (IsValid(CurrentWeapon))
	{
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	}
}

void UCombatComponent::FireTimerFinished()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(CurrentWeapon) || !IsValid(OwningPawn)) return;
	
	if (CurrentWeapon->Ammo <= 0 && CurrentReserveAmmo > 0 && OwningPawn->IsLocallyControlled())
	{
		Local_ReloadWeapon();
		Server_ReloadWeapon();
		return;
	}
	
	if (CurrentWeapon->WeaponStatus == EWeaponStatus::Firing)
	{
		CurrentWeapon->WeaponStatus = EWeaponStatus::Idle;
	}
	if (bTriggerPressed && CurrentWeapon->FireType == EFireType::Auto && CurrentWeapon->Ammo > 0)
	{
		Local_FireWeapon();
	}
}

void UCombatComponent::Multicast_FireWeapon_Implementation(const FHitResult& HitResult, int32 AuthAmmo)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (OwningPawn && OwningPawn->IsLocallyControlled())
	{
		CurrentWeapon->Rep_Fire(AuthAmmo);
	}
	else
	{
		ensure(IsValid(WeaponData));
		
		EPhysicalSurface ImpactSurfaceType = HitResult.PhysMaterial.IsValid(false) ? HitResult.PhysMaterial->SurfaceType.GetValue() : SurfaceType1;
		CurrentWeapon->LocalFire(HitResult.ImpactPoint, HitResult.ImpactNormal, ImpactSurfaceType, false);
		
		UAnimMontage* Montage3P = WeaponData->ThirdPersonMontages.FindChecked(CurrentWeapon->WeaponType).FireMontage;
		USkeletalMeshComponent* Mesh3P = IPlayerInterface::Execute_GetMesh3P(GetOwner());
		if (IsValid(Mesh3P) && IsValid(Montage3P))
		{
			Mesh3P->GetAnimInstance()->Montage_Play(Montage3P);
		}
	}
}

void UCombatComponent::Server_FireWeapon_Implementation(const FHitResult& HitResult)
{
	if (!IsValid(CurrentWeapon)) return;
	if (CurrentWeapon->Ammo <= 0) return;
	
	const bool bHit = IsValid(HitResult.GetActor()) && HitResult.GetActor()->Implements<UPlayerInterface>();
	const bool bHeadshot = bHit && HitResult.BoneName == FName("head");
	bool bLethal = false;	
	if (bHit)
	{
		bLethal = IPlayerInterface::Execute_DoDamage(HitResult.GetActor(), CurrentWeapon->Damage, GetOwner());
	}
	
	OnRoundReported.Broadcast(HitResult.GetActor(), GetOwner(), bHit, bHeadshot, bLethal);
	
	if (GetNetMode() != NM_ListenServer || !Cast<APawn>(GetOwner())->IsLocallyControlled())
	{
		CurrentWeapon->Auth_Fire();
	}
	Multicast_FireWeapon(HitResult, CurrentWeapon->Ammo);
}

void UCombatComponent::Initiate_FireWeapon_Released()
{
	bTriggerPressed = false;
}

void UCombatComponent::Initiate_ReloadWeapon()
{
	if (!IsValid(CurrentWeapon)) return;
	if (CurrentWeapon->WeaponStatus == EWeaponStatus::Cycling || CurrentWeapon->WeaponStatus == EWeaponStatus::Reloading) return;
	if (CurrentWeapon->Ammo == CurrentWeapon->MagCapacity) return;
	if (CurrentReserveAmmo <= 0) return;
	
	Local_ReloadWeapon();
	Server_ReloadWeapon();
}

void UCombatComponent::Local_ReloadWeapon()
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(CurrentWeapon) || !IsValid(OwningPawn)) return;
	ensure(WeaponData);
	
	const bool bIsLocallyControlled = OwningPawn->IsLocallyControlled();
	UAnimMontage* ReloadMontage = bIsLocallyControlled ? WeaponData->FirstPersonMontages.FindChecked(CurrentWeapon->WeaponType).ReloadMontage : WeaponData->ThirdPersonMontages.FindChecked(CurrentWeapon->WeaponType).ReloadMontage;
	USkeletalMeshComponent* Mesh = bIsLocallyControlled ? IPlayerInterface::Execute_GetMesh1P(OwningPawn) : IPlayerInterface::Execute_GetMesh3P(OwningPawn);
	if (IsValid(Mesh) && IsValid(ReloadMontage))
	{
		Mesh->GetAnimInstance()->Montage_Play(ReloadMontage);
	}
	
	UAnimMontage* WeaponReloadMontage = WeaponData->WeaponMontages.FindChecked(CurrentWeapon->WeaponType).ReloadMontage;
	USkeletalMeshComponent* WeaponMesh = bIsLocallyControlled ? CurrentWeapon->GetMesh1P() : CurrentWeapon->GetMesh3P();
	if (IsValid(WeaponMesh) && IsValid(WeaponReloadMontage))
	{
		WeaponMesh->GetAnimInstance()->Montage_Play(WeaponReloadMontage);
	}
	CurrentWeapon->WeaponStatus = EWeaponStatus::Reloading;
}

void UCombatComponent::Client_ReloadWeapon_Implementation(int32 NewWeaponAmmo, int32 NewCarriedAmmo)
{
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(CurrentWeapon) || !IsValid(OwningPawn)) return;
	
	if (OwningPawn->IsLocallyControlled())
	{
		CurrentWeapon->Ammo = NewWeaponAmmo;
		CurrentReserveAmmo = NewCarriedAmmo;
		
		OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterDynMatInst(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
		OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
	}
}

void UCombatComponent::Server_ReloadWeapon_Implementation()
{
	Multicast_ReloadWeapon();
}

void UCombatComponent::Multicast_ReloadWeapon_Implementation()
{
	Local_ReloadWeapon();
}

void UCombatComponent::Initiate_Aim_Pressed()
{
	Local_Aim(true);
	Server_Aim(true);
}

void UCombatComponent::Initiate_Aim_Released()
{
	Local_Aim(false);
	Server_Aim(false);
}

void UCombatComponent::Server_Aim_Implementation(const bool bPressed)
{
	Local_Aim(bPressed);
}

void UCombatComponent::Local_Aim(bool bPressed)
{
	bAiming = bPressed;
	OnAimingStatusChanged.Broadcast(bAiming);
}

void UCombatComponent::Equip(AWeapon* WeaponToEquip)
{
	CurrentWeapon = WeaponToEquip;
	CurrentWeapon->AttachToOwningPawn(Cast<APawn>(GetOwner()));
	
	CurrentReserveAmmo = ReserveAmmoMap.FindChecked(CurrentWeapon->WeaponType);
	OnCurrentReserveAmmoChanged.Broadcast(CurrentReserveAmmo, CurrentWeapon->Ammo, CurrentWeapon->WeaponIcon);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!IsValid(WeaponToEquip) || !IsValid(GetOwner())) return;
	if (GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		SetCurrentWeapon(CurrentWeapon, WeaponToEquip);
	}
	else
	{
		Server_EquipWeapon(WeaponToEquip);
	}
}

void UCombatComponent::Server_EquipWeapon_Implementation(AWeapon* WeaponToEquip)
{
	EquipWeapon(WeaponToEquip);
}

void UCombatComponent::SetCurrentWeapon(AWeapon* OldWeapon, AWeapon* NewWeapon)
{
	AWeapon* LocalLastWeapon = nullptr;
	if (IsValid(OldWeapon))
	{
		LocalLastWeapon = OldWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}
	if (IsValid(LocalLastWeapon))
	{
		LocalLastWeapon->DetachFromOwningPawn();
		LocalLastWeapon->WeaponStatus = EWeaponStatus::Unequipped;
	}
	
	CurrentWeapon = NewWeapon;
	APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwningPawn)) return;
	if (OwningPawn->HasAuthority() && IsValid(CurrentWeapon))
	{
		CurrentReserveAmmo = ReserveAmmoMap.FindChecked(CurrentWeapon->WeaponType);
	}
	if (!IsValid(CurrentWeapon)) return;
	CurrentWeapon->AttachToOwningPawn(OwningPawn);
	
	if (CurrentWeapon->Ammo == 0 && CurrentReserveAmmo > 0 && OwningPawn->IsLocallyControlled())
	{
		Local_ReloadWeapon();
		Server_ReloadWeapon();
	}
}

void UCombatComponent::SpawnInventory()
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority) return;
	
	for (const TSubclassOf<AWeapon>& WeaponClass : DefaultWeaponClasses)
	{
		AWeapon* NewWeapon = SpawnWeapon(WeaponClass);
		Inventory.AddUnique(NewWeapon);
		ReserveAmmoMap.Add(NewWeapon->WeaponType, NewWeapon->StartingCarriedAmmo);
	}
	
	if (Inventory.Num() > 0)
	{
		Equip(Inventory[0]);
		InitializeWeaponWidgets();
	}
}

void UCombatComponent::DestroyInventory()
{
	for (AWeapon* Weapon : Inventory)
	{
		if (IsValid(Weapon))
		{
			Weapon->Destroy();
		}
	}
}

void UCombatComponent::InitializeWeaponWidgets() const
{
	if (IsValid(CurrentWeapon))
	{
		OnReticleChanged.Broadcast(CurrentWeapon->GetReticleDynMatInst(), CurrentWeapon->ReticleParams, bHitPlayer);
		OnAmmoCounterChanged.Broadcast(CurrentWeapon->GetAmmoCounterDynMatInst(), CurrentWeapon->Ammo, CurrentWeapon->MagCapacity);
	}
}

void UCombatComponent::OnRep_CurrentWeapon(AWeapon* PreviousWeapon)
{
	SetCurrentWeapon(PreviousWeapon, CurrentWeapon);
	IPlayerInterface::Execute_WeaponReplicated(GetOwner());
	InitializeWeaponWidgets();
}

AWeapon* UCombatComponent::SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner)) return nullptr;
	if (Owner->GetLocalRole() < ROLE_Authority) return nullptr;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Cast<APawn>(Owner);
	SpawnParams.Owner = Owner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	return GetWorld()->SpawnActor<AWeapon>(WeaponClass, SpawnParams);
}
