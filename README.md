# UE4SpotifyWebAPI
This Unreal Engine 4 Project makes use of the Spotify WebAPI to control spotify playback and get data such as Song name, artists, Album and Playback time

How to Use
=

* Create yourself an app over at https://developer.spotify.com/dashboard/
* Set your Redirection URI to `127.0.0.1:8890`
* Fill into the `BP_SpotifyPlayerController` your `clientId` and `ClientSecret` in the defaults panel
* Press play and you're good to go! :-)

Features
-

 * See which song currently is playing (Songname, Artists, Album, Duration, Progress)
 * Set new Song (using Spotify Context urls - only track urls currently supported)
 * Get and Set the Volume on the playing device.
 * Get the Song-Artwork-Url
 * Seek through the song
 * Extend it urself using the [Spotify WebAPI](https://developer.spotify.com/documentation/web-api/reference/)


How it Works
=

[![Youtube Thumbnail](https://img.youtube.com/vi/ehNXvqhBl7Y/0.jpg)](https://www.youtube.com/watch?v=ehNXvqhBl7Y "Some Progress on the UnrealEngine-Spotify thing.")

I'll update this piece over time :-)
