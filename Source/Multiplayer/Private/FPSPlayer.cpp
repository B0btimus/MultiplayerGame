// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Multiplayer.h"
#include "FPSHealthComponent.h"
#include "FPSWeapon.h"
#include "MultiplayerGameModeBase.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AFPSPlayer::AFPSPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//create and setup FPS camera
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->bUsePawnControlRotation = true;
	CameraComp->SetupAttachment(GetMesh(), "headSocket");

	//create HealthComponent
	HealthComp = CreateDefaultSubobject<UFPSHealthComponent>(TEXT("HealthComp"));

	//disable capsule collision for weapon
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	//enable crouch
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	//enable jump
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;
}

// Called when the game starts or when spawned
void AFPSPlayer::BeginPlay()
{
	Super::BeginPlay();

	HealthComp->OnHealthChanged.AddDynamic(this, &AFPSPlayer::OnHealthChanged);

	if (Role == ROLE_Authority)
	{
		//spawn weapon
		FActorSpawnParameters ASP;
		ASP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentWeapon = GetWorld()->SpawnActor<AFPSWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, ASP);

		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "hand_rSocket");
		}
	}
}

// Called every frame
void AFPSPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFPSPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSPlayer::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AFPSPlayer::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &AFPSPlayer::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AFPSPlayer::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AFPSPlayer::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSPlayer::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSPlayer::StopFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSPlayer::Reload);
}

//called to get camera location for weapon line trace
FVector AFPSPlayer::GetPawnViewLocation() const
{
	if (CameraComp)
	{
		return CameraComp->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}

void AFPSPlayer::MoveForward(float Value)
{
	AddMovementInput(GetActorForwardVector() * Value);
}

void AFPSPlayer::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value);
}

void AFPSPlayer::BeginCrouch()
{
	Crouch();
}

void AFPSPlayer::EndCrouch()
{
	UnCrouch();
}

void AFPSPlayer::StartFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void AFPSPlayer::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void AFPSPlayer::Reload()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Reload();
	}
}

//only called on server beause healthcomponent is only set on server
void AFPSPlayer::OnHealthChanged(UFPSHealthComponent* OwningHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0 && !bDead)
	{
		bDead = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.0f);

		//AMultiplayerGameModeBase* Gm = (AMultiplayerGameModeBase*)GetWorld()->GetAuthGameMode();
		//Gm->RespawnPlayer();
	}
}

void AFPSPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSPlayer, CurrentWeapon);
	DOREPLIFETIME(AFPSPlayer, bDead);
}