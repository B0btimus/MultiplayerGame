// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSHealthComponent.h"
#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UFPSHealthComponent::UFPSHealthComponent()
{
	MaxHealth = 100.0f;

	SetIsReplicated(true);
}


// Called when the game starts
void UFPSHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// only bind function if this is the server
	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &UFPSHealthComponent::HandleTakeAnyDamage);
		}
	}

	Health = MaxHealth;
}

void UFPSHealthComponent::HandleTakeAnyDamage(AActor * DamagedActor, float Damage, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Damage <= 0.0f)
	{
		return;
	}

	//Update Health
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}

void UFPSHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UFPSHealthComponent, Health);
}