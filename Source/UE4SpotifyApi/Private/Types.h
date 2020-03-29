#pragma once


#include "CoreMinimal.h"
#include "Types.generated.h"

UENUM(BlueprintType)
enum ESpotifyAPIScopes
{
    UserLibraryRead,
    UserLibraryModify,
    PlaylistReadPrivate,
    PlaylistModifyPublic,
    PlaylistModifyPrivate,
    PlaylistReadCollaborative,
    UserReadRecentlyPlayed,
    UserTopRead,
    UserReadPrivate,
    UserReadEmail,
    UserReadBirthdate,
    Streaming,
    UserModifyPlaybackState,
    UserReadPlaybackState,
    UserReadCurrentlyPlaying,
    UserFollowModify,
    UserFollowRead
};


