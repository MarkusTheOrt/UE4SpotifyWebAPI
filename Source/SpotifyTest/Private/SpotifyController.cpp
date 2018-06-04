// Fill out your copyright notice in the Description page of Project Settings.

#include "SpotifyController.h"
#include "Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"


ASpotifyController::ASpotifyController()
  : PollingInterval(0.2)
  , Port(8890)
  , RedirectUri("http://127.0.0.1:8890")
  , AuthScopes({
      ESpotifyApiScopes::UserReadCurrentlyPlaying, 
      ESpotifyApiScopes::UserReadPlaybackState,
      ESpotifyApiScopes::UserModifyPlaybackState
    })
  , WebReturn(TEXT("HTTP/1.1 200 OK\r\nCache-Control: no-cache, private\r\n\r\n<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n<html><head>\r\n<title>Success!</title>\r\n</head><body>\r<h1>Success!</h1>\n<p>You can close this document now!</p>\r\n<script>window.close();</script></body></html>"))
{}


void ASpotifyController::GetAccessToken()
{
  auto Request = Http->CreateRequest();
  Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::OnAccessTokenReceived);
  Request->SetURL("https://accounts.spotify.com/api/token");
  Request->SetVerb("POST");
  Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
  const FString Content = "grant_type=authorization_code&redirect_uri=%s&client_id=%s&client_secret=%s&code=%s";
  Request->SetContentAsString(FString::Printf(*Content, *RedirectUri, *ClientId, *ClientSecret, *AuthKey));
  Request->ProcessRequest();
  
}



void ASpotifyController::OnAccessTokenReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
  if (bWasSuccessful)
  {

    TSharedPtr<FJsonObject> JsonObject;

    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {

      AccessToken = JsonObject->GetStringField("access_token");
      RefreshToken = JsonObject->GetStringField("refresh_token");
      GetWorldTimerManager().SetTimer(RefreshTokenTimer, this, &ThisClass::UseRefreshToken, JsonObject->GetIntegerField("expires_in") - 120, false);
      GetWorldTimerManager().SetTimer(PollingTimer, this, &ThisClass::FetchCurrentSong, PollingInterval, true);
      FetchCurrentSong();
    }
  }
}


void ASpotifyController::UseRefreshToken()
{
  UE_LOG(LogTemp, Warning, TEXT("Requesting Refresh Token Now!"));
  auto Request = Http->CreateRequest();
  Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::OnAccessTokenReceived);
  //This is the url on which to process the request
  Request->SetURL("https://accounts.spotify.com/api/token");
  Request->SetVerb("POST");
  Request->SetHeader("Content-Type", TEXT("application/x-www-form-urlencoded"));
  const FString Content = "grant_type=refresh_code&redirect_uri=%s&client_id=%s&client_secret=%s&code=%s";
  Request->SetContentAsString(FString::Printf(*Content, *RedirectUri, *ClientId, *ClientSecret, *RefreshToken));
  Request->ProcessRequest();
}


FString ASpotifyController::GetScopeValue(ESpotifyApiScopes Scope)
{
  switch (Scope)
  {
    case ESpotifyApiScopes::UserLibraryRead:
      return "user-library-read";
    case ESpotifyApiScopes::UserLibraryModify:
      return "user-library-modify";
    case ESpotifyApiScopes::PlaylistReadPrivate:
      return "playlist-read-private";
    case ESpotifyApiScopes::PlaylistModifyPrivate:
      return "playlist-modify-private";
    case ESpotifyApiScopes::PlaylistModifyPublic:
      return "playlist-modify-public";
    case ESpotifyApiScopes::PlaylistReadCollaborative:
      return "playlist-read-collaborative";
    case ESpotifyApiScopes::UserReadRecentlyPlayed:
      return "user-read-recently-played";
    case ESpotifyApiScopes::UserTopRead:
      return "user-top-read";
    case ESpotifyApiScopes::UserReadPrivate:
      return "user-read-private";
    case ESpotifyApiScopes::UserReadEmail:
      return "user-read-email";
    case ESpotifyApiScopes::UserReadBirthdate:
      return "user-read-birthdate";
    case ESpotifyApiScopes::Streaming:
      return "streaming";
    case ESpotifyApiScopes::UserModifyPlaybackState:
      return "user-modify-playback-state";
    case ESpotifyApiScopes::UserReadCurrentlyPlaying:
      return "user-read-currently-playing";
    case ESpotifyApiScopes::UserReadPlaybackState:
      return "user-read-playback-state";
    case ESpotifyApiScopes::UserFollowModify:
      return "user-follow-modify";
    case ESpotifyApiScopes::UserFollowRead:
      return "user-follow-read";
  }
  return "user-read-currently-playing";
}

FString ASpotifyController::BuildAuthURL()
{
  FString Scopes = "";
  for (ESpotifyApiScopes Scope : AuthScopes)
  {
    if (Scopes.Len() > 0)
      Scopes += " ";
    Scopes += GetScopeValue(Scope);
  }
  Scopes = FGenericPlatformHttp::UrlEncode(Scopes);
  //Hard-coded response_type - otherwise this doesn't work that way.
  const FString BaseUrl = "https://accounts.spotify.com/authorize?response_type=code&redirect_uri=%s&client_id=%s&scope=%s&state=%s";
  return FString::Printf(*BaseUrl, *RedirectUri, *ClientId, *Scopes, *State);
}

void ASpotifyController::FetchCurrentSong()
{
  if (AccessToken.Len() == 0) return;

  auto Request = Http->CreateRequest();
  Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::OnCurrentSongReceived);
  Request->SetURL("https://api.spotify.com/v1/me/player/currently-playing");
  Request->SetVerb("GET");
  Request->SetHeader("Content-Type", TEXT("application/json"));
  Request->SetHeader("Authorization", TEXT("Bearer ") + AccessToken);
  Request->ProcessRequest();

}

void ASpotifyController::PlaybackRequest(FString Url, FString Type)
{
  if (AccessToken.Len() == 0) return;

  auto Request = Http->CreateRequest();
  Request->SetURL(Url);
  Request->SetVerb(Type);
  Request->SetHeader("Content-Type", TEXT("application/json"));
  Request->SetHeader("Authorization", TEXT("Bearer ") + AccessToken);
  Request->ProcessRequest();
}

void ASpotifyController::RequestPause()
{
  //Pause/Play
  if (bIsPlaying)
  {
    PlaybackRequest("https://api.spotify.com/v1/me/player/pause", "PUT");
  }
  else 
  {
    PlaybackRequest("https://api.spotify.com/v1/me/player/play", "PUT");
  }
  
}

void ASpotifyController::RequestNextSong()
{
  PlaybackRequest("https://api.spotify.com/v1/me/player/next", "POST");
}

void ASpotifyController::RequestPrevSong()
{
  PlaybackRequest("https://api.spotify.com/v1/me/player/previous", "POST");
}

void ASpotifyController::OnCurrentSongReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
  if (bWasSuccessful)
  {

    TSharedPtr<FJsonObject> JsonObject;


    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());


    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
      //Lets get some of those JSON fields
      TSharedPtr<FJsonObject> Item = JsonObject->GetObjectField("item");

      bIsPlaying = JsonObject->GetBoolField("is_playing");

      TSharedPtr<FJsonObject> Album = Item->GetObjectField("album");

      auto Artists = Item->GetArrayField("artists");

      //Returning Artists
      FString ReturnArtists = "";

      //Show ALL the artists working on this song
      for (TSharedPtr<FJsonValue> Artist : Artists)
      {
        if (ReturnArtists.Len() > 0)
          ReturnArtists += ", ";
        ReturnArtists += Artist->AsObject()->GetStringField("name");

      }
      OnCurrentSong(
        Item->GetStringField("name"), 
        ReturnArtists,
        Album->GetStringField("name"), 
        Item->GetIntegerField("duration_ms"), 
        JsonObject->GetIntegerField("progress_ms"), 
        bIsPlaying
      );

    }

  }
}


void ASpotifyController::StartTCPListener()
{
  //Internal Loopback is 127.0.0.1 (localhost)
  FIPv4Endpoint Endpoint(FIPv4Address::InternalLoopback, Port);
  ListenerSocket = FTcpSocketBuilder(TEXT("ListenerSocket"))
    .AsReusable()
    .BoundToEndpoint(Endpoint)
    .Listening(8);

  int32 NewSize = 0;
  ListenerSocket->SetReceiveBufferSize(2 * 1024 * 1024, NewSize);

  GetWorldTimerManager().SetTimer(ListenerTimer, this, &ThisClass::TCPSocketListener, 0.01, true);

}

void ASpotifyController::TCPSocketListener()
{
  //Escape if theres no listener
  if (!ListenerSocket) return;
  auto SocketSubSys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
  TSharedRef<FInternetAddr> RemoteAddress = SocketSubSys->CreateInternetAddr();
  
  bool bPending;
  if (ListenerSocket->HasPendingConnection(bPending) && bPending)
  {

    if (ConnectionSocket)
    {
      ConnectionSocket->Close();
      SocketSubSys->DestroySocket(ConnectionSocket);
    }

    ConnectionSocket = ListenerSocket->Accept(*RemoteAddress, TEXT("ConnectionSocket"));
    if (ConnectionSocket != NULL)
    {
      ConnectionRemoteAddress = FIPv4Endpoint(RemoteAddress);
      GetWorldTimerManager().SetTimer(ConnectionTimer, this, &ThisClass::TCPConnectionHandler, 0.01, true);
    }
  }

}

void ASpotifyController::TCPConnectionHandler()
{
  //Escape if theres no Connection Socket
  if (!ConnectionSocket) return;
  auto SocketSubSys = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

  TArray<uint8> ReceivedData;
  
  uint32 Size;
  while (ConnectionSocket && ConnectionSocket->HasPendingData(Size))
  {
    ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));
    int32 Read = 0;
    ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

  }

  if (ReceivedData.Num() <= 0)
  {
    return;
  }

  //Convert our Received Message (plain HTTP Request) to an UnrealString
  const FString ReceivedString = FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(ReceivedData.GetData())));

  //Now lets send back a little page that says success (and closes if JS is enabled)
  TCHAR* serializedChar = WebReturn.GetCharArray().GetData();
  int32 size = FCString::Strlen(serializedChar);
  int32 sent = 0;

  ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);

  uint32 SentSize;
  while (ConnectionSocket && ConnectionSocket->HasPendingData(SentSize));

  //Request was successful (need to add fail checking everywhere... I'm Lazy)
  GetWorldTimerManager().ClearTimer(ConnectionTimer);
  //Close Connection, not needed anymore!
  ConnectionSocket->Close();
  SocketSubSys->DestroySocket(ConnectionSocket);

  //Lets get that AuthCode from our HTTP Response
  const FRegexPattern AuthFinder(TEXT("code=(.*)&"));
  FRegexMatcher AuthMatcher(AuthFinder, ReceivedString);
  if (AuthMatcher.FindNext())
  {
    AuthKey = AuthMatcher.GetCaptureGroup(1);

    //Great, now that we have the Auth Key we can get our Access- and Refresh Token from the API!
    GetAccessToken();
    
    //Also Destroy the Listener, not needed anymore at this point!
    GetWorldTimerManager().ClearTimer(ListenerTimer);
    if (ListenerSocket)
    {
      ListenerSocket->Close();
      SocketSubSys->DestroySocket(ListenerSocket);
    }
  }
}

void ASpotifyController::BeginDestroy()
{
  Super::BeginDestroy();
}

void ASpotifyController::BeginAuth()
{
  if (ClientId.Len() <= 0 || ClientSecret.Len() <= 0)
  {

    UE_LOG(LogTemp, Error, TEXT("Cannot begin auth process - ClientId And ClientSecret are not set!"));
    return;
  }

  StartTCPListener();
  Http = &FHttpModule::Get();
  //Wait for ListenerSocket to start up!
  while (!ListenerSocket);

  //Open up Spotify Auth Page for your app!
  FPlatformProcess::LaunchURL(*BuildAuthURL(), nullptr, nullptr);


}

