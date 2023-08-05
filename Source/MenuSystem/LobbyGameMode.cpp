// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"


//////////////////////////////////////////////////////////////////////////
// 플레이어가 로그인 합니다.
//////////////////////////////////////////////////////////////////////////
void ALobbyGameMode::PostLogin( APlayerController* newPlayer )
{
	Super::PostLogin( newPlayer );

	if ( GameState )
	{
		int32 numberOfPlayers = GameState.Get()->PlayerArray.Num();
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				60.f,
				FColor::Yellow,
				FString::Printf( TEXT( "Player in game: %d" ), numberOfPlayers ) );
		}

		APlayerState* playerState = newPlayer->GetPlayerState< APlayerState >();
		if ( playerState )
		{
			FString playerName = playerState->GetPlayerName();
			GEngine->AddOnScreenDebugMessage(
				1,
				60.f,
				FColor::Yellow,
				FString::Printf( TEXT( "%s has joined the game" ), *playerName ) );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// 플레이어가 로그아웃 합니다.
//////////////////////////////////////////////////////////////////////////
void ALobbyGameMode::Logout( AController* exiting )
{
	Super::Logout( exiting );

	APlayerState* playerState = exiting->GetPlayerState< APlayerState >();
	if ( playerState )
	{
		if ( GEngine )
		{
			int32 numberOfPlayers = GameState.Get()->PlayerArray.Num();

			GEngine->AddOnScreenDebugMessage(
				1,
				60.f,
				FColor::Yellow,
				FString::Printf( TEXT( "Player in game: %d" ), numberOfPlayers ) );
		}

		FString playerName = playerState->GetPlayerName();
		GEngine->AddOnScreenDebugMessage(
			1,
			60.f,
			FColor::Yellow,
			FString::Printf( TEXT( "%s has exited the game" ), *playerName ) );

	}
}
