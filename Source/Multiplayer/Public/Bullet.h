// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

UCLASS()
class MULTIPLAYER_API ABullet : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABullet();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	USceneComponent* RootComp;

	UPROPERTY(EditDefaultsOnly)
	class UBoxComponent* BulletCollision;

	UPROPERTY(EditDefaultsOnly)
	class UProjectileMovementComponent* BulletMovement;

	UStaticMeshComponent* BulletMesh;

	UPROPERTY(EditDefaultsOnly)
	class UParticleSystemComponent* BulletParticles;

	float Damage;
	AActor* WeaponOwner;
	TSubclassOf<UDamageType> Damagetype;

	//velocity of the bullet
	UPROPERTY(EditDefaultsOnly)
	float BulletSpeed;
	//this is the amount of seconds to wait until the bullet begins dropping
	UPROPERTY(EditDefaultsOnly)
	float GravityDelay;
	//the scale for the gravity acting upon the bullet
	UPROPERTY(EditDefaultsOnly)
	float GravityScale;

	UFUNCTION()
	void OnBulletHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpuls, const FHitResult& HitResult);

	//once we call this function, gravity will start effecting the bullet, making it drop.
	void ApplyGravity();

};
