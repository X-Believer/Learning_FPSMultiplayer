// Shuyang Xing All Rights Deserve


#include "Weapon/Weapon.h"

#include "KismetTraceUtils.h"
#include "FPS/FPS.h"
#include "Interfaces/PlayerInterface.h"
#include "Kismet/KismetMathLibrary.h"


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
	
	AimFOV = 65.0f;
	TraceRadius = 5.0f;
	FireTime = 0.1f;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
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

void AWeapon::WeaponTrace(FHitResult& OutHit, float TraceLength) const
{
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.AddIgnoredActor(GetOwner());
	
	FCollisionResponseParams ResponseParams;
	ResponseParams.CollisionResponse.SetAllChannels(ECR_Ignore);
	ResponseParams.CollisionResponse.SetResponse(ECollisionChannel::ECC_Pawn, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECollisionChannel::ECC_WorldStatic, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECollisionChannel::ECC_WorldDynamic, ECR_Block);
	ResponseParams.CollisionResponse.SetResponse(ECollisionChannel::ECC_PhysicsBody, ECR_Block);
	
	ensure(GetInstigator());
	if (APlayerController* PC = Cast<APlayerController>(GetInstigator()->GetController()); IsValid(PC))
	{
		FVector EyesWorldLocation;
		FRotator EyesWorldRotation;
		PC->GetActorEyesViewPoint(EyesWorldLocation, EyesWorldRotation);
		
		const FVector EyesWorldDirection = UKismetMathLibrary::GetForwardVector(EyesWorldRotation);
		const FVector StartLocation = EyesWorldLocation;
		const FVector EndLocation = EyesWorldLocation + (EyesWorldDirection * TraceLength);
		const bool bHit = GetWorld()->SweepSingleByChannel(
			OutHit,
			StartLocation,
			EndLocation,
			FQuat::Identity,
			FPSTraceChannels::ECC_Weapon,
			FCollisionShape::MakeSphere(TraceRadius),
			QueryParams,
			ResponseParams
		);
		
		if (!bHit)
		{
			OutHit.ImpactPoint = EndLocation;
			OutHit.ImpactNormal = -EyesWorldDirection;
		}
		// DrawDebugSphereTraceSingle(
		// 	GetWorld(),
		// 	StartLocation,
		// 	EndLocation,
		// 	TraceRadius,
		// 	EDrawDebugTrace::ForDuration,
		// 	bHit,
		// 	OutHit,
		// 	FLinearColor::Green,
		// 	FLinearColor::Red,
		// 	5.0f
		// );
	}
}

void AWeapon::LocalFire(const FVector& ImpactPoint, const FVector& ImpactNormal, TEnumAsByte<EPhysicalSurface> ImpactSurfaceType, bool bIsFirstPerson)
{
	FireEffects(ImpactPoint, ImpactNormal, ImpactSurfaceType, bIsFirstPerson);
	
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
