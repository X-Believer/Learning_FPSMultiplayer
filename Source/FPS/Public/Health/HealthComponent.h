// Shuyang Xing All Rights Deserve

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FHealthChanged, UHealthComponent*, HealthComp, float, OldValue, float, NewValue, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeathStarted);

UENUM(BlueprintType)
enum class EDeathState : uint8
{
	NotDead,
	DeadStarted,
	DeadFinished
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintPure, Category = "FPS|Health")
	static UHealthComponent* FindHealthComponent(const AActor* Actor) { return IsValid(Actor) ? Actor->FindComponentByClass<UHealthComponent>() : nullptr; }
	
	UFUNCTION(BlueprintCallable, Category = "FPS|Health")
	float GetHealthNormalized() const;
	
	UPROPERTY(ReplicatedUsing=OnRep_DeathState, BlueprintReadOnly, Category="FPS|Health")
	EDeathState DeathState;
	
	UPROPERTY(BlueprintAssignable)
	FDeathStarted OnDeathStarted;
	
	UPROPERTY(ReplicatedUsing=OnRep_Health, EditDefaultsOnly, Category="FPS|Health")
	float Health;
	
	UPROPERTY(ReplicatedUsing=OnRep_MaxHealth, EditDefaultsOnly, Category="FPS|Health")
	float MaxHealth;
	
	UPROPERTY(BlueprintAssignable)
	FHealthChanged OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable)
	FHealthChanged OnMaxHealthChanged;
	
	bool ChangeHealthByAmount(float Amount, AActor* Instigator);
	void ChangeMaxHealthByAmount(float Amount, AActor* Instigator);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnRep_DeathState(EDeathState OldDeathState);
	
	UFUNCTION()
	void OnRep_Health(float OldValue);
	
	UFUNCTION()
	void OnRep_MaxHealth(float OldValue);
	
private:
	void StartDeath();
};
