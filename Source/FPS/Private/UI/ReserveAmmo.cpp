// Shuyang Xing All Rights Deserve


#include "ReserveAmmo.h"

#include "Character/ShooterCharacter.h"
#include "Combat/CombatComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Weapon/Weapon.h"

void UReserveAmmo::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Image_WeaponIcon->SetRenderOpacity(0.0f);
	Text_Ammo->SetRenderOpacity(0.0f);
	
	GetOwningPlayer()->OnPossessedPawnChanged.AddDynamic(this, &UReserveAmmo::OnPossessedPawnChanged);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayerPawn());
	if (!IsValid(ShooterCharacter)) return;
	
	OnPossessedPawnChanged(nullptr, ShooterCharacter);
	
	if (ShooterCharacter->HasWeaponFirstReplicated())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (IsValid(Weapon))
		{
			OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetReserveAmmo(ShooterCharacter), Weapon->Ammo, Weapon->WeaponIcon);
		}
	}
	else
	{
		ShooterCharacter->OnWeaponFirstReplicated.AddDynamic(this, &ThisClass::OnWeaponFirstReplicated);
	}
	if (ShooterCharacter->HasAuthority())
	{
		AWeapon* Weapon = IPlayerInterface::Execute_GetCurrentWeapon(ShooterCharacter);
		if (!IsValid(Weapon)) return;

		OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetReserveAmmo(ShooterCharacter), Weapon->Ammo, Weapon->WeaponIcon);
	}
}

void UReserveAmmo::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UCombatComponent* OldPawnCombat = UCombatComponent::FindCombatComponent(OldPawn);
	if (IsValid(OldPawnCombat))
	{
		OldPawnCombat->OnCurrentReserveAmmoChanged.RemoveDynamic(this, &ThisClass::OnCurrentReserveAmmoChanged);
		OldPawnCombat->OnRoundFired.RemoveDynamic(this, &ThisClass::OnRoundFired);
	}
	
	UCombatComponent* NewPawnCombat = UCombatComponent::FindCombatComponent(NewPawn);
	if (IsValid(NewPawnCombat))
	{
		Image_WeaponIcon->SetRenderOpacity(1.0f);
		Text_Ammo->SetRenderOpacity(1.0f);
		NewPawnCombat->OnCurrentReserveAmmoChanged.AddDynamic(this, &ThisClass::OnCurrentReserveAmmoChanged);
	}
}

void UReserveAmmo::OnCurrentReserveAmmoChanged(int32 RoundsInReserve, int32 RoundsInWeapon, UMaterialInterface* WeaponIconMaterial)
{
	if (IsValid(WeaponIconMaterial))
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(WeaponIconMaterial);
		if (IsValid(Image_WeaponIcon))
		{
			Image_WeaponIcon->SetBrush(Brush);
		}
	}
	
	if (IsValid(Text_Ammo))
	{
		FText AmmoText = FText::Format(NSLOCTEXT("AmmoText", "AmmoKey", "{0}/{1}"), FText::AsNumber(RoundsInWeapon), FText::AsNumber(RoundsInReserve));
		Text_Ammo->SetText(AmmoText);
	}
}

void UReserveAmmo::OnRoundFired(int32 RoundsCurrent, int32 RoundsMax, int32 RoundsInReserve)
{
	if (IsValid(Text_Ammo))
	{
		FText AmmoText = FText::Format(NSLOCTEXT("AmmoText", "AmmoKey", "{0}/{1}"), FText::AsNumber(RoundsCurrent), FText::AsNumber(RoundsInReserve));
		Text_Ammo->SetText(AmmoText);
	}
}

void UReserveAmmo::OnWeaponFirstReplicated(AWeapon* Weapon)
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetOwningPlayerPawn());
	if (!IsValid(ShooterCharacter)) return;
	
	OnCurrentReserveAmmoChanged(IPlayerInterface::Execute_GetReserveAmmo(ShooterCharacter), Weapon->Ammo, Weapon->WeaponIcon);
}
