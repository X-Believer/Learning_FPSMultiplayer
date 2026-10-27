// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ShooterTypes/ShooterTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EFireType : uint8
{
	Auto UMETA(DisplayName = "Automatic"),
	SemiAuto UMETA(DisplayName = "Semi-Automatic"),
};

UENUM(BlueprintType)
enum class EWeaponStatus : uint8
{
	Idle,
	Firing,
	Reloading,
	Cycling,
	Unequipped
};

UCLASS()
class FPS_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();
	
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
	UMaterialInstanceDynamic* GetReticleDynMatInst();
	UMaterialInstanceDynamic* GetAmmoCounterDynMatInst();
	
	void AttachToOwningPawn(APawn* Pawn) const;
	void DetachFromOwningPawn() const;
	void WeaponTrace(FHitResult& OutHit, float TraceLength) const;
	
	void LocalFire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson);
	void Auth_Fire();
	void Rep_Fire(int32 AuthAmmo);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|WeaponType")
	FGameplayTag WeaponType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS|Aiming")
	float AimFOV;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FPS|Trace")
	float TraceRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|FireType")
	EFireType FireType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|FireType")
	float FireTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Ammo")
	int32 MagCapacity;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Ammo")
	int32 Ammo;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FPS|Ammo")
	int32 StartingCarriedAmmo;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Reticle")
	FReticleParams ReticleParams;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Icon")
	TObjectPtr<UMaterialInterface> WeaponIcon;
	
	EWeaponStatus WeaponStatus;
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void FireEffects(const FVector& ImpactPoint, const FVector& ImpactNormal, EPhysicalSurface ImpactSurfaceType, bool bIsFirstPerson);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<USkeletalMeshComponent> Mesh1P;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FPS|Weapon")
	TObjectPtr<USkeletalMeshComponent> Mesh3P;
	
private:
	
	void SetWeaponVisibility(const APawn* OwningPawn) const;
	
	int32 Sequence;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TObjectPtr<UMaterialInterface> ReticleMaterial;
	
	UPROPERTY(EditDefaultsOnly, Category="FPS|Weapon")
	TObjectPtr<UMaterialInterface> AmmoCounterMaterial;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynMatInst_Reticle;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynMatInst_AmmoCounter;
};


