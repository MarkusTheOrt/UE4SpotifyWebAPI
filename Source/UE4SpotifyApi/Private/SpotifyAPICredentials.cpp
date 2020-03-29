// Fill out your copyright notice in the Description page of Project Settings.


#include "SpotifyAPICredentials.h"
#include "Kismet/GameplayStatics.h"


void USpotifyAPICredentials::FillInCredentials(const FString& InPubKey, const FString& InSecKey)
{
	PubKey = InPubKey;
	SecKey = InSecKey;

	UGameplayStatics::SaveGameToSlot(this, TEXT("SpotifySEC"), 0);
}

void USpotifyAPICredentials::GetCredentials(FString& OutPubKey, FString& OutSecKey) const
{
	OutPubKey = PubKey;
	OutSecKey = SecKey;
}
