// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

class FSocket;

/**
 * 
 */
class UE4SPOTIFYAPI_API ISpotifyAPIClient : public FRunnable
{


protected:

    static  ISpotifyAPIClient* Runnable;
	
    ISpotifyAPIClient();

    uint16 Port = 8064;

    FSocket* ListenerSocket;
    FSocket* ConnectionSocket;
	
    FRunnableThread* Thread;

    FThreadSafeCounter StopTaskCounter;

    FString Code;
	
    bool bFinished;
	
public:

	virtual ~ISpotifyAPIClient();

    static ISpotifyAPIClient* Get()
    {
        static ISpotifyAPIClient* Client;
        return Client;
    }

    static ISpotifyAPIClient* GetStarted();

    FString GetCode() const;
	
	bool IsFinished() const
	{
        return bFinished;
	}

    virtual bool Init();
    virtual uint32 Run();
    virtual void Stop();
	
    void EnsureCompletion();
	
    static void Shutdown();

    static bool IsThreadFinished();
};

