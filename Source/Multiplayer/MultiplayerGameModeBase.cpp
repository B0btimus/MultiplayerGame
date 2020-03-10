// Fill out your copyright notice in the Description page of Project Settings.
//GameMode only runs on server

#include "MultiplayerGameModeBase.h"
#include "FPSPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMultiplayerGameModeBase::AMultiplayerGameModeBase()
{
	//default spawn delay
	RespawnTime = 5.0f;
}

void AMultiplayerGameModeBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMultiplayerGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RespawnPlayer();
}
 
void AMultiplayerGameModeBase::RespawnPlayer()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() == nullptr)//if there is no pawn player is dead
		{
			if ((RespawnTime - GetWorld()->DeltaTimeSeconds) <= 0)
			{
				RestartPlayer(PC);
			}
		}
	}
}

