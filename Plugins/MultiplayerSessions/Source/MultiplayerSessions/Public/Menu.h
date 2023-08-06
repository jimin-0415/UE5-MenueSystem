// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"


class UButton;
class UMultiPlayerSessionsSubsystem;


/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

/// Subsystem
private:
	/// The subsystem designed to handl all online session functionality
	UMultiPlayerSessionsSubsystem* m_MultiPlayerSessionSubsystem;

	/// 연결가능한 Connection 수
	int32 m_NumPublicConnections{ 4 };

	/// MatchType 정의
	FString m_MatchType{ TEXT( "FreeForAll" ) };

	/// 로비 패스 정의
	FString m_PathToLobby{ TEXT( "" ) };


/// UI Object
private:
	/// 바인딩되는 버튼의 이름은 블루프린트에 정의된 이름과 동일해야 한다.
	// UPROPERTY( meta = ( BindWidget ) ) 

	/// Host 버튼
	UPROPERTY( meta = ( BindWidget ) )
		UButton* m_HostButton;

	/// Join 버튼
	UPROPERTY( meta = ( BindWidget ) )
		UButton* m_JoinButton;


public:
	/// 메뉴 초기 설정을 합니다.
	UFUNCTION( BlueprintCallable )
		void MenuSetup(
		int32 numberOfPublicConnections = 4,
		FString typeOfMatch = FString( TEXT( "FreeForAll" ) ),
		FString lobbyPath = FString( TEXT( "/Game/ThirdPerson/Maps/Lobby" ) )
		);


protected:
	/// 초기화를 진행합니다.
	virtual bool Initialize() override;

	/// 다른 레벨로 이동할 경우 현재 Level World 를 파괴합니다.
	virtual void NativeDestruct() override;


/// Callbacks for the custom delegates on th MultiplayerSessionsSubsystem
protected:
	/// 세션이 생성 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
	UFUNCTION()
	void OnCreateSession( bool bWasSuccessful );

	/// 세션 찾기 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
	void OnFindSessions( const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful );
	
	/// 세션 합류 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
	void OnJoinSession( EOnJoinSessionCompleteResult::Type result );
	
	/// 세션 파괴 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
	UFUNCTION()
	void OnDestroySession( bool bWasSuccessful );
	
	/// 세선 시작 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
	UFUNCTION()
	void OnStartSession( bool bWasSuccessful );


private:
	/// Host 버튼을 클릭합니다.
	UFUNCTION()
	void HostButtonClicked();

	/// Join 버튼을 클릭합니다.
	UFUNCTION()
	void JoinButtonClicked();

	/// 메뉴 키조작을 합니다.
	void MenuTearDown();
};
