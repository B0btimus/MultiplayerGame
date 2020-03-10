// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

// Sets default values
ABullet::ABullet()
{
	BulletSpeed = 4000.0f;
	GravityDelay = 0.1f;
	GravityScale = 3.5f;

	BulletCollision = CreateDefaultSubobject<UBoxComponent>(FName("BulletCollision"));
	BulletCollision->InitBoxExtent(FVector(2.0f));
	BulletCollision->BodyInstance.SetCollisionProfileName("Projectile");
	BulletCollision->OnComponentHit.AddDynamic(this, &ABullet::OnBulletHit);
	SetRootComponent(BulletCollision);

	BulletMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("BulletMovement"));
	BulletMovement->UpdatedComponent = BulletCollision;
	BulletMovement->InitialSpeed = BulletSpeed;
	BulletMovement->bShouldBounce = false;
	BulletMovement->ProjectileGravityScale = 0.0f;


	InitialLifeSpan = 5.0f;
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();
	
	//after <GravityDelay> amount of time, make the bullet start dropping
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ABullet::ApplyGravity, GravityDelay, false);
}

void ABullet::ApplyGravity()
{
	BulletMovement->ProjectileGravityScale = GravityScale;
}

void ABullet::OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpuls, const FHitResult& HitResult)
{
	UE_LOG(LogTemp, Warning, TEXT("hit"));
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Hit"));
	EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
	if (SurfaceType == SurfaceType2)
	{
		Damage *= 4.0f;
		UE_LOG(LogTemp, Warning, TEXT("headshot"));
	}

	UGameplayStatics::ApplyPointDamage(OtherActor, 10.0f, GetActorForwardVector().BackwardVector, HitResult, WeaponOwner->GetInstigatorController(), this, Damagetype);

	Destroy();
}

