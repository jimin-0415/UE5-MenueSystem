// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiPlayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"


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
	if ( !m_SessionInterface.IsValid() )
		return;

	// 이미 세션이 존재할 경우 삭제 후 다시 설정.
	auto existingSession = m_SessionInterface->GetNamedSession( NAME_GameSession );
	if ( nullptr != existingSession )
	{
		m_IsCreateSessionOnDestroy = true;
		m_LastNumPublicConnections = numPublicConnections;
		m_LastMatchType = matchType;

		// 세션 파괴 후 생성을 할경우 파괴 요청 시 서버와의 통신 딜레이 시간때문에, 이미 존재하는 세션이라. 문제가 발생함
		// 세션 파괴 완료 후 세션 시작하도록 처리.
		DestroySession();
	}
		

	// Store the delegate in a FDelegateHandle so We can later remove it for the delegate list
	m_CreateSessionCompleteDelegateHandle = 
		m_SessionInterface->AddOnCreateSessionCompleteDelegate_Handle( m_CreateSessionCompleteDelegate );

	m_LastSessionSettings = MakeShareable( new FOnlineSessionSettings() );
	
	// 테스트 용도일경우  SubSystemName == NuLL, 
	// 테스트 아닐경우 Ex SubSystemName == Steam - ex
	m_LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	// Connection Count
	m_LastSessionSettings->NumPublicConnections = numPublicConnections;

	m_LastSessionSettings->bAllowJoinInProgress	 = true;
	m_LastSessionSettings->bAllowJoinViaPresence = true;
	m_LastSessionSettings->bShouldAdvertise		 = true;   //광고
	m_LastSessionSettings->bUsesPresence		 = true;
	m_LastSessionSettings->bUseLobbiesIfAvailable= true;
	m_LastSessionSettings->BuildUniqueId		 = 1;		// 유니크 아이디 설정

	m_LastSessionSettings->Set( FName( "MatchType" ), matchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing );

	// 월드로부터 로컬플레이어 정보를 가져온다. 각 로컬 플레이어는 고유의 Id값을 가진다.
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	// 세션 생성 
	if ( !m_SessionInterface->CreateSession( *localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *m_LastSessionSettings ) )
	{
		// 세션 생성이 실패할 경우.
		m_SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle( m_CreateSessionCompleteDelegateHandle );

		// Broadcast our own custom delegate
		m_MultiplayerOnCreateSessionComplete.Broadcast( false );
	}
}

////////////////////////////////////////////////////////////////////////////
/// 세션을 찾습니다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::FindSessions( int32 maxSearchResults )
{
	if ( !m_SessionInterface.IsValid() )
		return;

	m_FindSessionCompleteDelegateHandle 
		= m_SessionInterface->AddOnFindSessionsCompleteDelegate_Handle( m_FindSessionCompleteDelegate );

	m_LastSessionSearch = MakeShareable( new FOnlineSessionSearch() );
	m_LastSessionSearch->MaxSearchResults = maxSearchResults;
	m_LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	m_LastSessionSearch->QuerySettings.Set( SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals ); // 세션 검색 쿼리 세팅 

	// 월드로부터 로컬플레이어 정보를 가져온다. 각 로컬 플레이어는 고유의 Id값을 가진다.
	//Each player that is active on the current client/listen server has a LocalPlayer.
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	// 세션 찾기
	if ( !m_SessionInterface->FindSessions( *localPlayer->GetPreferredUniqueNetId(), m_LastSessionSearch.ToSharedRef() ) )
	{
		m_SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle( m_FindSessionCompleteDelegateHandle );
		
		//BroadCast Delegate
		m_MultiplayerOnFindSessionsComplete.Broadcast( TArray<FOnlineSessionSearchResult>(), false );
	}
}

////////////////////////////////////////////////////////////////////////////
/// 세션에 참가합니다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::JoinSession( const FOnlineSessionSearchResult& sessionResult )
{
	if ( !m_SessionInterface.IsValid() )
	{
		m_MultiplayerOnJoinSessionComplete.Broadcast( EOnJoinSessionCompleteResult::UnknownError );
		return;
	}
	
	m_JoinSessionCompleteDelegateHandle 
		= m_SessionInterface->AddOnJoinSessionCompleteDelegate_Handle( m_JoinSessionCompleteDelegate );

	// 로컬 플레이어 정보
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	// 세션 참가
	if ( !m_SessionInterface->JoinSession( *localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, sessionResult ) )
	{
		m_SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle( m_JoinSessionCompleteDelegateHandle );

		m_MultiplayerOnJoinSessionComplete.Broadcast( EOnJoinSessionCompleteResult::UnknownError );
	}
}

////////////////////////////////////////////////////////////////////////////
/// 세션을 파괴합니다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::DestroySession()
{
	if ( !m_SessionInterface.IsValid() )
	{
		m_MultiplayerOnDestroySessionComplete.Broadcast( false );
		return;
	}

	m_DestroySessionCompleteDelegateHandle =
		m_SessionInterface->AddOnDestroySessionCompleteDelegate_Handle( m_DestroySessionCompleteDelegate );

	if ( !m_SessionInterface->DestroySession( NAME_GameSession ) )
	{
		m_SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle( m_DestroySessionCompleteDelegateHandle );
		m_MultiplayerOnDestroySessionComplete.Broadcast( false );
	}
}

////////////////////////////////////////////////////////////////////////////
/// 세션을 시작합니다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::StartSession()
{
}

////////////////////////////////////////////////////////////////////////////
/// 멀티플레이어 세션 생성 완료 대리자를 반환한다.
////////////////////////////////////////////////////////////////////////////
FMultiplayerOnCreateSessionComplete& UMultiPlayerSessionsSubsystem::GetMultiplayerOnCreateSessionComplete()
{
	return m_MultiplayerOnCreateSessionComplete;
}

////////////////////////////////////////////////////////////////////////////
/// 멀티플레이어 세션 검색 완료 대리자를 반환한다.
////////////////////////////////////////////////////////////////////////////
FMultiplayerOnFindSessionsComplete& UMultiPlayerSessionsSubsystem::GetMultiplayerOnFindSessionsComplete()
{
	return m_MultiplayerOnFindSessionsComplete;
}

////////////////////////////////////////////////////////////////////////////
/// 멀티플레이어 세션 참가 완료 대리자를 반환한다.
////////////////////////////////////////////////////////////////////////////
FMultiplayerOnJoinSessionComplete& UMultiPlayerSessionsSubsystem::GetMultiplayerOnJoinSessionComplete()
{
	return m_MultiplayerOnJoinSessionComplete;
}

////////////////////////////////////////////////////////////////////////////
/// 멀티플레이어 세션 파괴 완료 대리자를 반환한다.
////////////////////////////////////////////////////////////////////////////
FMultiplayerOnDestroySessionComplete& UMultiPlayerSessionsSubsystem::GetMultiplayerOnDestroySessionComplete()
{
	return m_MultiplayerOnDestroySessionComplete;
}

////////////////////////////////////////////////////////////////////////////
/// 멀티플레이어 세션 시작 완료 대리자를 반환한다.
////////////////////////////////////////////////////////////////////////////
FMultiplayerOnStartSessionComplete& UMultiPlayerSessionsSubsystem::GetMultiplayerOnStartSessionComplete()
{
	return m_MultiplayerOnStartSessionComplete;
}

////////////////////////////////////////////////////////////////////////////
/// 세션 생성이 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnCreateSessionComplete( FName sessionName, bool bWasSuccessful )
{
	if ( m_SessionInterface )
	{
		m_SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle( m_CreateSessionCompleteDelegateHandle );
	}

	// Broadcast our own custom delegate
	m_MultiplayerOnCreateSessionComplete.Broadcast( bWasSuccessful );
}

////////////////////////////////////////////////////////////////////////////
/// 세션 검색이 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnFindSessionsComplete( bool bwasSuccessful )
{
	if ( m_SessionInterface )
	{
		m_SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle( m_FindSessionCompleteDelegateHandle );
	}

	if ( m_LastSessionSearch->SearchResults.Num() <= 0 )
	{
		// 찾은 세션 정보가 없을경우 실패 처리.
		m_MultiplayerOnFindSessionsComplete.Broadcast( TArray<FOnlineSessionSearchResult>(), false );
		return;
	}

	// Broadcast our own custom delegate
	m_MultiplayerOnFindSessionsComplete.Broadcast( m_LastSessionSearch->SearchResults, bwasSuccessful );
}

////////////////////////////////////////////////////////////////////////////
/// 세션 합류가 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnJoinSessionComplete( FName sessionName, EOnJoinSessionCompleteResult::Type result )
{
	if ( m_SessionInterface )
	{
		m_SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle( m_JoinSessionCompleteDelegateHandle );
	}

	m_MultiplayerOnJoinSessionComplete.Broadcast( result );
}

////////////////////////////////////////////////////////////////////////////
/// 세션 파괴가 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnDestroySessionComplete( FName sessionName, bool bwasSuccessful )
{
	if ( m_SessionInterface )
	{
		m_SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle( m_DestroySessionCompleteDelegateHandle );
	}

	if ( bwasSuccessful && m_IsCreateSessionOnDestroy )
	{
		m_IsCreateSessionOnDestroy = false;
		CreateSession( m_LastNumPublicConnections, m_LastMatchType );
	}

	m_MultiplayerOnDestroySessionComplete.Broadcast( bwasSuccessful );
}

////////////////////////////////////////////////////////////////////////////
/// 세션 시작이 완료되었을 때 처리한다.
////////////////////////////////////////////////////////////////////////////
void UMultiPlayerSessionsSubsystem::OnStartSessionComplete( FName sessionName, bool bwasSuccessful )
{
}