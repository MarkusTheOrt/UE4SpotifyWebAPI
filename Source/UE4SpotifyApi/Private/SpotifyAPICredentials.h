// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SpotifyAPICredentials.generated.h"


/**
 * 
 */
UCLASS()
class USpotifyAPICredentials : public USaveGame
{
	GENERATED_BODY()


private:

	UPROPERTY(SaveGame)
	FString PubKey;

	UPROPERTY(SaveGame)
	FString SecKey;

public:

	void FillInCredentials(const FString& InPubKey, const FString& InSecKey);

	void GetCredentials(FString& OutPubKey, FString& OutSecKey) const;
	
};
