// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FPlayerAnims
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponData|PlayerAnims")
	TObjectPtr<UAnimSequence> IdleAnim = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponData|PlayerAnims")
	TObjectPtr<UAnimSequence> AimAnim = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponData|PlayerAnims")
	TObjectPtr<UAnimSequence> CrouchAnim = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponData|PlayerAnims")
	TObjectPtr<UAnimSequence> SprintAnim = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponData|PlayerAnims")
	TObjectPtr<UBlendSpace> AimOffset_Hip;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponData|PlayerAnims")
	TObjectPtr<UBlendSpace> AimOffset_Aim;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponData|PlayerAnims")
	TObjectPtr<UBlendSpace> Strafe_Standing;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponData|PlayerAnims")
	TObjectPtr<UBlendSpace> Strafe_Crouching;
	
};

USTRUCT(BlueprintType)
struct FMontageData
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|WeaponData|MontageData")
	TObjectPtr<UAnimMontage> EquipMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|WeaponData|MontageData")
	TObjectPtr<UAnimMontage> FireMontage = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|WeaponData|MontageData")
	TObjectPtr<UAnimMontage> ReloadMontage = nullptr;
};

UCLASS()
class FPS_API UWeaponData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="FPS|WeaponData|Weapons")
	TMap<FGameplayTag, FName> GripPoints;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|WeaponData|FirstPerson")
	TMap<FGameplayTag, FPlayerAnims> FirstPersonAnims;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|WeaponData|ThirdPerson")
	TMap<FGameplayTag, FPlayerAnims> ThirdPersonAnims;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|WeaponData|FirstPerson")
	TMap<FGameplayTag, FMontageData> FirstPersonMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|WeaponData|ThirdPerson")
	TMap<FGameplayTag, FMontageData> ThirdPersonMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="FPS|WeaponData|Weapons")
	TMap<FGameplayTag, FMontageData> WeaponMontages;
};
