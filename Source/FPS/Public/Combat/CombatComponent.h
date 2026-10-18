// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


class AWeapon;
class UWeaponData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	// Cycle to next weapon
	void Initiate_CycleWeapon();
	void Initiate_FireWeapon_Pressed();
	void Initiate_FireWeapon_Released();
	void Initiate_ReloadWeapon();
	void Initiate_Aim_Pressed();
	void Initiate_Aim_Released();
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TObjectPtr<UWeaponData> WeaponData;
	
	UFUNCTION()
	void Equip(AWeapon* WeaponToEquip);
	UFUNCTION()
	void SpawnInventory();
	UFUNCTION()
	void DestroyInventory();
	
protected:
	
private:
	
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<AWeapon> CurrentWeapon;
	
	UFUNCTION()
	void OnRep_CurrentWeapon(AWeapon* PreviousWeapon);
	
	UPROPERTY(Transient, Replicated)
	TArray<AWeapon*> Inventory;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TArray<TSubclassOf<AWeapon>> DefaultWeaponClasses;
	
	UFUNCTION()
	AWeapon* SpawnWeapon(TSubclassOf<AWeapon> WeaponClass) const;
};
