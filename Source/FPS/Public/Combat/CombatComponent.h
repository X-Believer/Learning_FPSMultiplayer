// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class AWeapon;
class UWeaponData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReticleChanged, UMaterialInstanceDynamic*, ReticleDynMatInst, const FReticleParams&, ReticleParams, bool, bCurrentlyTargetingPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAmmoCounterChanged, UMaterialInstanceDynamic*, AmmoCounterDynMatInst, int32, CurrentAmmo, int32, MagCapacity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FRoundFired, int32, RoundsCurrent, int32, RoundsMax, int32, RoundsInReserve);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimingStatusChanged, bool, bAiming);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTargetingPlayerStatusChanged, bool, bTargetingPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCurrentREserveAmmoChanged, int32, RoundsInReserve, int32, RoundsInWeapon, UMaterialInterface*, WeaponIconMaterial);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintPure, Category="FPS|Combat")
	static UCombatComponent* FindCombatComponent(const AActor* Actor) { return IsValid(Actor) ? Actor->FindComponentByClass<UCombatComponent>() : nullptr; }
	
	// Cycle to next weapon
	void Initiate_CycleWeapon();
	void Initiate_FireWeapon_Pressed();
	void Initiate_FireWeapon_Released();
	void Initiate_ReloadWeapon();
	void Initiate_Aim_Pressed();
	void Initiate_Aim_Released();
	UFUNCTION(BlueprintCallable)
	void Notify_CycleWeapons();
	UFUNCTION(BlueprintCallable)
	void Notify_ReloadWeapon();
	
	void AddAmmo(const FGameplayTag& WeaponType, int32 AmmoToAdd);
	
	UPROPERTY(BlueprintAssignable)
	FAmmoCounterChanged OnAmmoCounterChanged;
	UPROPERTY(BlueprintAssignable)
	FReticleChanged OnReticleChanged;
	UPROPERTY(BlueprintAssignable)
	FAimingStatusChanged OnAimingStatusChanged;
	UPROPERTY(BlueprintAssignable)
	FTargetingPlayerStatusChanged OnTargetingPlayerStatusChanged;
	UPROPERTY(BlueprintAssignable)
	FRoundFired OnRoundFired;
	UPROPERTY(BlueprintAssignable)
	FCurrentREserveAmmoChanged OnCurrentReserveAmmoChanged;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Weapon")
	TObjectPtr<UWeaponData> WeaponData;
	
	UFUNCTION()
	void Equip(AWeapon* WeaponToEquip);
	UFUNCTION()
	void EquipWeapon(AWeapon* WeaponToEquip);
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AWeapon* WeaponToEquip);
	UFUNCTION()
	void SpawnInventory();
	UFUNCTION()
	void DestroyInventory();
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentReserveAmmo)
	int32 CurrentReserveAmmo;
	
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bAiming;
	
	AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }
	
	void InitializeWeaponWidgets() const;
	
protected:
		
	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AWeapon> CurrentWeapon;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	float TraceLength;
	
	UFUNCTION()
	void BlendOut_CycleWeapon(UAnimMontage* Montage, bool bInterrupted);
	
private:
	bool bTriggerPressed;
	FTimerHandle FireTimer;
	bool bHitPlayerLastFrame;
	bool bHitPlayer;
	TMap<FGameplayTag, int32> ReserveAmmoMap;
	int32 Local_WeaponIndex;
	
	int32 AdvanceWeaponIndex();
	void Local_CycleWeapon(int32 WeaponIndex);
	UFUNCTION(Server, Reliable)
	void Server_CycleWeapon(int32 WeaponIndex);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CycleWeapon(int32 WeaponIndex);
	
	void SetCurrentWeapon(AWeapon* OldWeapon, AWeapon* NewWeapon);
	
	void FireTimerFinished();
	
	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* PreviousWeapon);
	
	UPROPERTY(Transient, Replicated)
	TArray<AWeapon*> Inventory;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TArray<TSubclassOf<AWeapon>> DefaultWeaponClasses;
	
	UFUNCTION()
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
	
	UFUNCTION(Server, Reliable)
	void Server_Aim(bool bPressed);
	UFUNCTION()
	void Local_Aim(bool bPressed);
	
	UFUNCTION(Server, Reliable)
	void Server_FireWeapon(const FHitResult& HitResult);
	UFUNCTION()
	void Local_FireWeapon();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireWeapon(const FHitResult& HitResult, int32 AuthAmmo);
	
	UFUNCTION()
	void OnRep_CurrentReserveAmmo();
	
	void Local_ReloadWeapon();
	
	UFUNCTION(Server, Reliable)
	void Server_ReloadWeapon();
	
	UFUNCTION(Client, Reliable)
	void Client_ReloadWeapon(int32 NewWeaponAmmo, int32 NewCarriedAmmo);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReloadWeapon();
};
