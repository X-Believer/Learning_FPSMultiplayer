// Shuyang Xing All Rights Deserve


#include "Weapon/Weapon.h"

#include "Interfaces/PlayerInterface.h"


AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh1P"));
	Mesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh1P->CastShadow = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->SetHiddenInGame(true, true);
	SetRootComponent(Mesh1P);
	
	Mesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh3P"));
	Mesh3P->SetupAttachment(RootComponent);
	Mesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	Mesh3P->CastShadow = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->SetHiddenInGame(true, true);
}

void AWeapon::OnRep_Instigator()
{
	Super::OnRep_Instigator();
	AttachToOwningPawn();
}

void AWeapon::AttachToOwningPawn() const
{
	const APawn* OwningPawn = GetInstigator();
	if (!IsValid(OwningPawn) || !OwningPawn->Implements<UPlayerInterface>()) return;
	SetWeaponVisibility(OwningPawn);
	
	const FName AttachPoint = IPlayerInterface::Execute_GetWeaponAttachPoint(OwningPawn, WeaponType);
	USkeletalMeshComponent* PawnMesh1P = IPlayerInterface::Execute_GetMesh1P(OwningPawn);
	USkeletalMeshComponent* PawnMesh3P = IPlayerInterface::Execute_GetMesh3P(OwningPawn);
	
	Mesh1P->AttachToComponent(PawnMesh1P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
	Mesh3P->AttachToComponent(PawnMesh3P, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::SetWeaponVisibility(const APawn* OwningPawn) const
{
	if (OwningPawn->IsLocallyControlled())
	{
		Mesh1P->SetHiddenInGame(false, true);
		Mesh3P->SetHiddenInGame(true, true);
	}
	else
	{
		Mesh1P->SetHiddenInGame(true, true);
		Mesh3P->SetHiddenInGame(false, true);
	}
}