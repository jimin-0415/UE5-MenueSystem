// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiPlayerSessionsSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiPlayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/// 생성자
	UMultiPlayerSessionsSubsystem();

/// To Handle session functionality. The Menu class will call these
public:
	/// 세션을 생성합니다.
	void CreateSession( int32 numPublicConnections, FString matchType );

	/// 세션을 찾습니다.
	void FindSessions( int32 maxSearchResults );

	/// 세션에 참가합니다.
	void JoinSession( const FOnlineSessionSearchResult& sessionResult );

	/// 세션을 파괴합니다.
	void DestroySession();

	/// 세션을 시작합니다.
	void StartSession();

/// Internal callbacks for the delegates we'll add to the OnlineSession Interface delegate list.
/// This don't need to be called outside this class
protected:
	/// 세션 생성이 완료되었을 때 처리한다.
	void OnCreateSessionComplete( FName sessionName, bool bWasSuccessful );

	/// 세션 검색이 완료되었을 때 처리한다.
	void OnFindSessionsComplete(bool bwasSuccessful);

	/// 세션 합류가 완료되었을 때 처리한다.
	void OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);

	/// 세션 파괴가 완료되었을 때 처리한다.
	void OnDestroySessionComplete(FName sessionName, bool bwasSuccessful);

	/// 세션 시작이 완료되었을 때 처리한다.
	void OnStartSessionComplete(FName sessionName, bool bwasSuccessful);

private:
	/// 온라인 세션 인터페이스
	IOnlineSessionPtr m_SessionInterface;

	/// 마지막 세션 세팅 정의
	TSharedPtr< FOnlineSessionSettings > m_lastSessionSettings;

/// To add to the Online Session Interface delegate list.
/// We`ll bind our MultiPlayerSessionsSubsystem internal callbacks to these.
private:
	/// 세션 생성 완료 대리자
	FOnCreateSessionCompleteDelegate m_CreateSessionCompleteDelegate;
	
	/// 세션 생성 완료 대리자 핸들
	FDelegateHandle m_CreateSessionCompleteDelegateHandle;

	/// 세션 검색 완료 대리자
	FOnFindSessionsCompleteDelegate m_FindSessionCompleteDelegate;
	
	//// 세션 검색 완료 대리자 핸들
	FDelegateHandle m_FindSessionCompleteDelegateHandle;

	/// 세션 참가 완료 대리자
	FOnJoinSessionCompleteDelegate m_JoinSessionCompleteDelegate;
	
	/// 세션 참가 완료 대리자 핸들
	FDelegateHandle m_JoinSessionCompleteDelegateHandle;

	/// 세션 파괴 완료 대리자
	FOnDestroySessionCompleteDelegate m_DestroySessionCompleteDelegate;
	
	/// 세션 파괴 완료 대리자 핸들
	FDelegateHandle m_DestroySessionCompleteDelegateHandle;

	/// 세션 시작 완료 대리자
	FOnStartSessionCompleteDelegate m_StartSessionCompleteDelegate;

	/// 세션 시작 완료 대리자 핸들
	FDelegateHandle m_StartSessionCompleteDelegateHandle;
};
