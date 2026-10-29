// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/PlayerInterface.h"
#include "ShooterTypes/ShooterTypes.h"
#include "ShooterCharacter.generated.h"

class UInputAction;
class UCombatComponent;
class UCameraComponent;
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponFirstReplicated, AWeapon*, Weapon);

UCLASS()
class FPS_API AShooterCharacter : public ACharacter, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	/* Player Interface */
	virtual FName GetWeaponAttachPoint_Implementation(const FGameplayTag& WeaponType) const override;
	virtual USkeletalMeshComponent* GetMesh1P_Implementation() const override;
	virtual USkeletalMeshComponent* GetMesh3P_Implementation() const override;
	virtual void WeaponReplicated_Implementation() override;
	virtual AWeapon* GetCurrentWeapon_Implementation() const override;
	virtual int32 GetReserveAmmo_Implementation() const override;
	virtual void Notify_CycleWeapons_Implementation() override;
	virtual void Notify_ReloadWeapon_Implementation() override;
	virtual void AddAmmo_Implementation(const FGameplayTag& WeaponType, int32 AmmoToAdd) override;
	/* End Player Interface */
	
	UFUNCTION(BlueprintCallable)
	FRotator GetFixedAimRotation() const;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|FABRIK")
	FTransform FABRIK_SocketTransform;
	
	UFUNCTION(BlueprintCallable)
	bool HasCurrentWeapon() const;
	
	UPROPERTY(BlueprintAssignable)
	FWeaponFirstReplicated OnWeaponFirstReplicated;

	bool HasWeaponFirstReplicated() const;
protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Combat")
	TObjectPtr<UCombatComponent> CombatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="FPS|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|Aiming")
	float DefaultFOV;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnAim(bool bIsAiming);
	
private:	

	void Input_CycleWeapon();
	void Input_FireWeapon_Pressed();
	void Input_FireWeapon_Released();
	void Input_ReloadWeapon();
	void Input_Aim_Pressed();
	void Input_Aim_Released();
	
	void CalculateFABRIKSocketTransform();
	void CalculateTurnInPlaceParams(float DeltaTime);
	void TurnInPlace(float DeltaTime);
	
	bool bWeaponFirstReplicated;
	
	FRotator StartingAimingRotation;
	float InterpAO_Yaw;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float AO_Yaw;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|Strafing", meta=(AllowPrivateAccess="true"))
	float MovementOffsetYaw;
	
	UPROPERTY(BlueprintReadOnly, Category="FPS|TurnInPlace", meta=(AllowPrivateAccess="true"))
	ETurnInPlace TurningStatus;
	
	// 1st person mesh (arms), visible only to the owning player.
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	UPROPERTY(VisibleAnywhere) 
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> CycleWeaponAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> FireWeaponAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> ReloadWeaponAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> AimWeaponAction;
	
};
