// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

class AWeapon;
struct FGameplayTag;

UINTERFACE()
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FPS_API IPlayerInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FPS|PlayerInterface")
	FName GetWeaponAttachPoint(const FGameplayTag& WeaponType) const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FPS|PlayerInterface")
	USkeletalMeshComponent* GetMesh1P() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FPS|PlayerInterface")
	USkeletalMeshComponent* GetMesh3P() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FPS|PlayerInterface")
	void WeaponReplicated();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="FPS|PlayerInterface")
	AWeapon* GetCurrentWeapon() const;
};
