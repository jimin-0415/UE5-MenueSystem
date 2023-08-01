// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiPlayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

////////////////////////////////////////////////////////////////////////////
/// 생성자
////////////////////////////////////////////////////////////////////////////
UMultiPlayerSessionsSubsystem::UMultiPlayerSessionsSubsystem()
	: m_CreateSessionCompleteDelegate	( FOnCreateSessionCompleteDelegate::CreateUObject( this, &ThisClass::OnCreateSessionComplete	) ),
	  m_FindSessionCompleteDelegate		( FOnFindSessionsCompleteDelegate::CreateUObject( this, &ThisClass::OnFindSessionsComplete		) ),
	  m_JoinSessionCompleteDelegate		( FOnJoinSessionCompleteDelegate::CreateUObject( this, &ThisClass::OnJoinSessionComplete		) ),
 	  m_DestroySessionCompleteDelegate	( FOnDestroySessionCompleteDelegate::CreateUObject( this, &ThisClass::OnDestroySessionComplete  ) ),
 	  m_StartSessionCompleteDelegate	( FOnStartSessionCompleteDelegate::CreateUObject( this, &ThisClass::OnStartSessionComplete		) )
{
	IOnlineSubsystem* subSystem = IOnlineSubsystem::Get();
	if ( subSystem )
	{
		// 서브 시스템으로 부터 세션 관리가 가능한 세션 인터페이스 정보를 가져온다.
		m_SessionInterface = subSystem->GetSessionInterface();
	}
}

////////////////////////////////////////////////////////////////////////////
/// 세션을 생성합니다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::CreateSession( int32 numPublicConnections, FString matchType )
{
}

////////////////////////////////////////////////////////////////////////////
/// 세션을 찾습니다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::FindSessions( int32 maxSearchResults )
{
}

////////////////////////////////////////////////////////////////////////////
/// 세션에 참가합니다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::JoinSession( const FOnlineSessionSearchResult& sessionResult )
{
}

////////////////////////////////////////////////////////////////////////////
/// 세션을 파괴합니다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::DestroySession()
{
}

////////////////////////////////////////////////////////////////////////////
/// 세션을 시작합니다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::StartSession()
{
}

////////////////////////////////////////////////////////////////////////////
/// 세션 생성이 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnCreateSessionComplete( FName sessionName, bool bWasSuccessful )
{
}

////////////////////////////////////////////////////////////////////////////
/// 세션 검색이 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnFindSessionsComplete( bool bwasSuccessful )
{
}

////////////////////////////////////////////////////////////////////////////
/// 세션 합류가 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnJoinSessionComplete( FName sessionName, EOnJoinSessionCompleteResult::Type result )
{
}

////////////////////////////////////////////////////////////////////////////
/// 세션 파괴가 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnDestroySessionComplete( FName sessionName, bool bwasSuccessful )
{
}

////////////////////////////////////////////////////////////////////////////
/// 세션 시작이 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnStartSessionComplete( FName sessionName, bool bwasSuccessful )
{
}