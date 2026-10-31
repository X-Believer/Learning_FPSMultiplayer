// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerStateReplicated);

/**
 * 
 */
UCLASS()
class FPS_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AShooterPlayerController();
	virtual void OnRep_PlayerState() override;
	bool bPawnAlive;
	
	UPROPERTY(BlueprintAssignable)
	FPlayerStateReplicated OnPlayerStateReplicated;
	
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	
private:
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputMappingContext> ShooterIMC;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditAnywhere, Category = "FPS|Input")
	TObjectPtr<UInputAction> CrouchAction;
	
	void Input_Crouch();
	void Input_Jump();
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_Look(const FInputActionValue& InputActionValue);
};
