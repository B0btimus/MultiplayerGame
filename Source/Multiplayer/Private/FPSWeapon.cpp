// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSWeapon.h"
#include "Bullet.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "Multiplayer.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AFPSWeapon::AFPSWeapon()
{
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	Damage = 20.0f;

	FireRate = 600;

	//set defaults for ammonation
	MaxAmmo = 150;
	MagazineSize = 25;
	Ammo = MagazineSize;

	ReloadTime = 2.5f;

	SetReplicates(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AFPSWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / FireRate;
}

void AFPSWeapon::FireWeapon()
{
	if (Role < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* WeaponOwner = GetOwner();
	if (WeaponOwner && Ammo>0)
	{
		FVector EyeLocation; //camera position
		FRotator EyeRotation; //camera rotation
		WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection * 10000); //set endpoint of trace

		//define collision query parameter
		FCollisionQueryParams CQP;
		CQP.AddIgnoredActor(WeaponOwner);
		CQP.AddIgnoredActor(this);
		CQP.bReturnPhysicalMaterial = true;
		CQP.bTraceComplex = true; //gives exact result where trace hits something

		FVector TracerEndPoint = TraceEnd;

		EPhysicalSurface SurfaceType = SurfaceType_Default;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, COLLISION_WEAPON, CQP))
		{
			//Blocking hit process damage
			AActor* HitActor = Hit.GetActor();

			SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			float ActualDamage = Damage;
			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 4.0f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, WeaponOwner->GetInstigatorController(), this, Damagetype);

			PlayFireEffects(Hit.ImpactPoint);

			//UE_LOG(LogClass, Warning, TEXT("Hit actor is %s"), HitActor->GetName());

			TracerEndPoint = Hit.ImpactPoint;

			HitScanTrace.SurfaceType = SurfaceType;
		}


		//play effects and sound
		PlayFireEffects(TracerEndPoint);
		if (Role == ROLE_Authority)
		{
			HitScanTrace.TraceEnd = TracerEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;
		}

		LastShot = GetWorld()->TimeSeconds;

		Ammo--;
	}
}

void AFPSWeapon::ServerFire_Implementation()
{
	FireWeapon();
}

//validate the code and check if there is somthing wrong (for anti-cheat)
bool AFPSWeapon::ServerFire_Validate()
{
	return true;
}

void AFPSWeapon::OnRep_HitScanTrace()
{
	//play fire effects
	PlayFireEffects(HitScanTrace.TraceEnd);

	PlayImpactEffects(HitScanTrace.SurfaceType, HitScanTrace.TraceEnd);
}

void AFPSWeapon::StartFire()
{
	float FirstDelay =FMath::Max(LastShot + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(Timer_TimeBetweenShots, this, &AFPSWeapon::FireWeapon, TimeBetweenShots, true, FirstDelay);
}

void AFPSWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(Timer_TimeBetweenShots);
}

void AFPSWeapon::Reload()
{
	if (Ammo < MagazineSize)
	{
		StopFire();
		if (MaxAmmo > MagazineSize)
		{
			Ammo = MagazineSize;
			MaxAmmo -= MagazineSize;
		}
		else if(MaxAmmo!=0)
		{
			Ammo = MaxAmmo;
			MaxAmmo = 0;
		}
	}
}


void AFPSWeapon::PlayFireEffects(FVector TraceEnd)
{
	if (MuzzleEffect)//muzzle flash
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)//smokebeam of projectile
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}

	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}
}

void AFPSWeapon::PlayImpactEffects(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;
	//set impact effect for surfaces
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;
	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		FVector ShotDirection = ImpactPoint - MeshComp->GetSocketLocation(MuzzleSocketName);
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DefaultImpactEffect, ImpactPoint, ShotDirection.Rotation());
	}
}

void AFPSWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFPSWeapon, HitScanTrace,COND_SkipOwner);
}