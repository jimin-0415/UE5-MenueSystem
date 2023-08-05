// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MENUSYSTEM_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	/// 플레이어가 로그인 합니다.
	virtual void PostLogin( APlayerController* newPlayer ) override;

	/// 플레이어가 로그아웃 합니다.
	virtual void Logout( AController* exiting ) override;
};
