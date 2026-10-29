// Shuyang Xing All Rights Deserve


#include "Health/HealthComponent.h"

#include "Net/UnrealNetwork.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	DeathState = EDeathState::NotDead;
	SetIsReplicatedByDefault(true);
	Health = 100.0f;
	MaxHealth = 100.0f;
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, DeathState);
	DOREPLIFETIME_CONDITION(UHealthComponent, Health, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UHealthComponent, MaxHealth, COND_OwnerOnly);
}

float UHealthComponent::GetHealthNormalized() const
{
	return FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
}


void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

bool UHealthComponent::ChangeHealthByAmount(float Amount, AActor* Instigator)
{
	float OldValue = Health;
	Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(this, OldValue, Health, Instigator);
	
	if (Health <= 0.0f && DeathState == EDeathState::NotDead)
	{
		StartDeath();
	}
	
	return false;
}

void UHealthComponent::StartDeath()
{
	if (DeathState != EDeathState::NotDead) return;
	
	DeathState = EDeathState::DeadStarted;
	OnDeathStarted.Broadcast();
	GetOwner()->ForceNetUpdate();
}

void UHealthComponent::ChangeMaxHealthByAmount(float Amount, AActor* Instigator)
{
	float OldValue = MaxHealth;
	MaxHealth += Amount;
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, Instigator);
}

void UHealthComponent::OnRep_DeathState(EDeathState OldDeathState)
{
	if (DeathState == EDeathState::DeadStarted)
	{
		OnDeathStarted.Broadcast();
	}
}

void UHealthComponent::OnRep_Health(float OldValue)
{
	OnHealthChanged.Broadcast(this, OldValue, Health, nullptr);
}

void UHealthComponent::OnRep_MaxHealth(float OldValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, MaxHealth, nullptr);
}
