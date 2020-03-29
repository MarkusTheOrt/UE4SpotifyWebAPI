// Fill out your copyright notice in the Description page of Project Settings.



#include "UE4SpotifyApi/Public/SpotifyAPIClient.h"
#include "SpotifyAPICredentials.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Runtime/Networking/Public/Networking.h"
#include "Runtime/Sockets/Public/Sockets.h"
#include "TimerManager.h"

ISpotifyAPIClient* ISpotifyAPIClient::Runnable = NULL;

ISpotifyAPIClient::ISpotifyAPIClient()
	: StopTaskCounter(0)
	, bFinished(false)
{
	UE_LOG(LogTemp, Warning, TEXT("Constructor"));

	Thread = FRunnableThread::Create(this, TEXT("SpotifyListenClient"), 0, TPri_BelowNormal);
}

ISpotifyAPIClient::~ISpotifyAPIClient()
{
	delete Thread;
	Thread = NULL;
	if(ListenerSocket)
	{
		ListenerSocket->Close();
	}
}

ISpotifyAPIClient* ISpotifyAPIClient::GetStarted()
{
	if(!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new ISpotifyAPIClient();
	}
	return Runnable;
}

FString ISpotifyAPIClient::GetCode() const
{
	return Code;
}

bool ISpotifyAPIClient::Init()
{
	//USpotifyAPICredentials* Credentials = Cast<USpotifyAPICredentials>(UGameplayStatics::LoadGameFromSlot(TEXT("SpotifySEC"), 0));
	//if (!Credentials) return false;
	const FString PubKey = "1032673ea07c4e3baf71241e45257144";
	const FString SecKey = "1032673ea07c4e3baf71241e45257144";
	//Credentials->GetCredentials(PubKey, SecKey);
	//Credentials->BeginDestroy();
	//Credentials = nullptr;
	const FIPv4Endpoint Endpoint(FIPv4Address::InternalLoopback, Port);

	ListenerSocket = FTcpSocketBuilder(TEXT("ListenerSocket"))
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.Listening(8)
		.Build();

	int NewSize = 0;
	ListenerSocket->SetReceiveBufferSize(2 * 1024 * 1024, NewSize);
	FPlatformProcess::LaunchURL(TEXT("https://accounts.spotify.com/authorize?response_type=code&redirect_uri=http://127.0.0.1:8064&client_id=1032673ea07c4e3baf71241e45257144&scope=user-modify-playback-state&state="), TEXT(""), nullptr);
	return true;
}

uint32 ISpotifyAPIClient::Run()
{
	FPlatformProcess::Sleep(0.03);
	if (!ListenerSocket) return 0;
	auto SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	const TSharedRef<FInternetAddr> RemoteAddress = SocketSubsystem->CreateInternetAddr();
	UE_LOG(LogTemp, Log, TEXT("TEST"));
	bool bPending;
	while(ListenerSocket && ListenerSocket->HasPendingConnection(bPending) && !bPending)
	{
		FPlatformProcess::Sleep(0.03);
		if (StopTaskCounter.GetValue() > 0) {
			ListenerSocket->Close();
			SocketSubsystem->DestroySocket(ListenerSocket);
			ListenerSocket = nullptr;
			return 0;
		}
	}
	if (ListenerSocket->HasPendingConnection(bPending) && bPending)
	{
		
		/*if (ConnectionSocket != NULL)
		{
			ConnectionSocket->Close();
			SocketSubsystem->DestroySocket(ConnectionSocket);
		}*/

		ConnectionSocket = ListenerSocket->Accept(*RemoteAddress, TEXT("ConnectionSocket"));
		
		if(ConnectionSocket)
		{
			FPlatformProcess::Sleep(0.03);
			uint32 Size;
			TArray<uint8> ReceivedData;
			
			while(ConnectionSocket->HasPendingData(Size))
			{	
				ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));
				int32 Read = 0;
				ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);
				FPlatformProcess::Sleep(0.06);
			}

			if(ReceivedData.Num() <= 0)
			{
				return 0;
			}

			const FString ReceivedString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));

			UE_LOG(LogTemp, Warning, TEXT("Received: %s"), *ReceivedString);

			TCHAR* SerializedChar = FString(TEXT("HTTP/1.1 200 OK\r\nCache-Control: no-cache, private\r\n\r\n<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n<html><head>\r\n<title>Success!</title>\r\n</head><body>\r<h1>Success!</h1>\n<p>You can close this document now!</p>\r\n<script>window.close();</script></body></html>")).GetCharArray().GetData();
			int32 size = FCString::Strlen(SerializedChar);
			int32 sent = 0;

			
			
			while(ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(SerializedChar), size, sent) && size - sent > 0);
			uint32 SizeSent;

			while (ConnectionSocket && ConnectionSocket->HasPendingData(SizeSent));

			ConnectionSocket->Close();
			SocketSubsystem->DestroySocket(ConnectionSocket);
			FPlatformProcess::Sleep(0.03);

			bFinished = true;

			ListenerSocket->Close();
			SocketSubsystem->DestroySocket(ListenerSocket);

			const FRegexPattern CodePattern("=(\\S+)&");
			FRegexMatcher CodeMatcher(CodePattern, ReceivedString);

			CodeMatcher.FindNext();
			Code = CodeMatcher.GetCaptureGroup(1);
			
			return 1;
			
		}
	}

	if(ConnectionSocket && bFinished)
	{
		ConnectionSocket->Close();
		SocketSubsystem->DestroySocket(ConnectionSocket);
	}
	if(ListenerSocket && bFinished)
	{
		ListenerSocket->Close();
		SocketSubsystem->DestroySocket(ListenerSocket);
	}

	return 0;
}

void ISpotifyAPIClient::Stop()
{
	StopTaskCounter.Increment();
}

void ISpotifyAPIClient::EnsureCompletion()
{
	Stop();
	Thread->WaitForCompletion();
}

void ISpotifyAPIClient::Shutdown()
{
	if(Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = NULL;
	}
}

bool ISpotifyAPIClient::IsThreadFinished()
{
	if (Runnable) return Runnable->IsFinished();
	return true;
}
