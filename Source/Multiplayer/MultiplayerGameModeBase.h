// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiplayerGameModeBase.generated.h"

/**
 * 
 */

class AFPSPlayer;

UCLASS()
class MULTIPLAYER_API AMultiplayerGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<AFPSPlayer> PlayerClass;

	//respawn delay
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float RespawnTime;
	FTimerHandle Timer_RespawnTime;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	AMultiplayerGameModeBase();
	
	void RespawnPlayer();
};
