// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiPlayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"


////////////////////////////////////////////////////////////////////////////
/// 메뉴 초기 설정을 합니다.
////////////////////////////////////////////////////////////////////////////
void UMenu::MenuSetup( int32 numberOfPublicConnections, FString typeOfMatch, FString lobbyPath )
{
	m_PathToLobby = FString::Printf( TEXT( "%s?listen" ), *lobbyPath );
	m_NumPublicConnections = numberOfPublicConnections;
	m_MatchType = typeOfMatch;

	AddToViewport();
	SetVisibility( ESlateVisibility::Visible );
	bIsFocusable = true;

	UWorld* world = GetWorld();
	if ( world )
	{
		APlayerController* playerController = world->GetFirstPlayerController();
		if ( playerController )
		{
			// Menu에서 사용할 Ui의 조작 방식을 설정합니다.
			FInputModeUIOnly inputModeData;
			inputModeData.SetWidgetToFocus( TakeWidget() );
			inputModeData.SetLockMouseToViewportBehavior( EMouseLockMode::DoNotLock );
			playerController->SetInputMode( inputModeData );
			playerController->SetShowMouseCursor( true );
		}
	}

	// GameInstace 를 통해서 SubSystem 정보를 가져옵니다.
	UGameInstance* gameInstance = GetGameInstance();
	if ( gameInstance )
	{
		m_MultiPlayerSessionSubsystem = gameInstance->GetSubsystem< UMultiPlayerSessionsSubsystem >();
	}

	if ( m_MultiPlayerSessionSubsystem )
	{
		// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam 에 바인딩 되는 함수는 UFUNC() 매크로 선언이 필요하다.
		// 대리자 클래스에 함수를 매핑합니다.
		m_MultiPlayerSessionSubsystem->GetMultiplayerOnCreateSessionComplete().AddDynamic(this, &ThisClass::OnCreateSession);
		m_MultiPlayerSessionSubsystem->GetMultiplayerOnFindSessionsComplete().AddUObject( this, &ThisClass::OnFindSessions );
		m_MultiPlayerSessionSubsystem->GetMultiplayerOnJoinSessionComplete().AddUObject(this, &ThisClass::OnJoinSession);
		m_MultiPlayerSessionSubsystem->GetMultiplayerOnDestroySessionComplete().AddDynamic( this, &ThisClass::OnDestroySession );
		m_MultiPlayerSessionSubsystem->GetMultiplayerOnStartSessionComplete().AddDynamic( this, &ThisClass::OnStartSession );
	}
}

////////////////////////////////////////////////////////////////////////////
/// 초기화를 진행합니다.
////////////////////////////////////////////////////////////////////////////
bool UMenu::Initialize()
{
	if ( !Super::Initialize() )
		return false;

	if ( m_HostButton )
	{
		m_HostButton->OnClicked.AddDynamic( this, &ThisClass::HostButtonClicked );
	}

	if ( m_JoinButton )
	{
		m_JoinButton->OnClicked.AddDynamic( this, &ThisClass::JoinButtonClicked );
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////
/// 다른 레벨로 이동할 경우 현재 Level World 를 파괴합니다.
////////////////////////////////////////////////////////////////////////////
void UMenu::NativeDestruct()
{
	MenuTearDown();

	Super::NativeDestruct();
}

////////////////////////////////////////////////////////////////////////////
/// 세션이 생성 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
////////////////////////////////////////////////////////////////////////////
void UMenu::OnCreateSession( bool bWasSuccessful )
{
	if ( bWasSuccessful )
	{
		// 정상적으로 Session이 만들어졌다고 판단되면 World 이동
		UWorld* world = GetWorld();
		if ( world )
		{
			world->ServerTravel( m_PathToLobby );
		}

		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString( TEXT( "Session Create Successfully" ) ) );
		}
	}
	else
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString( TEXT( "Failed to Create Session" ) ) );
		}

		m_HostButton->SetIsEnabled( true );
	}
}

////////////////////////////////////////////////////////////////////////////
/// 세션 찾기 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
////////////////////////////////////////////////////////////////////////////
void UMenu::OnFindSessions( const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful )
{
	if ( nullptr == m_MultiPlayerSessionSubsystem )
		return;

	for ( auto& result : sessionResults )
	{
		FString settingsValue;
		result.Session.SessionSettings.Get( FName( "MatchType" ), settingsValue );
		if ( m_MatchType == settingsValue )
		{
			m_MultiPlayerSessionSubsystem->JoinSession( result );
			return;
		}
	}

	if ( !bWasSuccessful || sessionResults.Num() == 0 )
	{
		m_JoinButton->SetIsEnabled( true );

		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString( TEXT( "Failed to Find Session" ) ) );
		}
	}
}

////////////////////////////////////////////////////////////////////////////
/// 세션 합류 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
////////////////////////////////////////////////////////////////////////////
void UMenu::OnJoinSession( EOnJoinSessionCompleteResult::Type result )
{
	// 일단 Success 가 아니면 반환
	if ( EOnJoinSessionCompleteResult::Success != result )
		return;

	IOnlineSubsystem* subSystem = IOnlineSubsystem::Get();
	if ( subSystem )
	{
		IOnlineSessionPtr sessionInterface = subSystem->GetSessionInterface();
		if ( sessionInterface.IsValid() )
		{
			FString address;
			sessionInterface->GetResolvedConnectString( NAME_GameSession, address );

			APlayerController* playerController = GetGameInstance()->GetFirstLocalPlayerController();
			if ( playerController )
			{
				// absolute travel 유영의 주소를 전달한다.
				playerController->ClientTravel( address, ETravelType::TRAVEL_Absolute );
			}
			
		}
	}

	if ( result != EOnJoinSessionCompleteResult::Success )
	{
		m_JoinButton->SetIsEnabled( true );
	}
}

////////////////////////////////////////////////////////////////////////////
/// 세션 파괴 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
////////////////////////////////////////////////////////////////////////////
void UMenu::OnDestroySession( bool bWasSuccessful )
{
}

////////////////////////////////////////////////////////////////////////////
/// 세선 시작 결과를 처리한다. [ Delegator 로부터 전달받아 호출된 함수 ]
////////////////////////////////////////////////////////////////////////////
void UMenu::OnStartSession( bool bWasSuccessful )
{
}

////////////////////////////////////////////////////////////////////////////
/// Host 버튼을 클릭합니다.
////////////////////////////////////////////////////////////////////////////
void UMenu::HostButtonClicked()
{
	// 클릭을 하면 버튼을 비활성화 한다.
	m_HostButton->SetIsEnabled( false );

	if ( m_MultiPlayerSessionSubsystem )
	{
		/// TODO. 일단 들어오는지 검사하기 위해서 임시로 적용.
		m_MultiPlayerSessionSubsystem->CreateSession( m_NumPublicConnections, m_MatchType );
	}

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString( TEXT( "Host Button Clicked" ) ) );
	}
}

////////////////////////////////////////////////////////////////////////////
/// Join 버튼을 클릭합니다..
////////////////////////////////////////////////////////////////////////////
void UMenu::JoinButtonClicked()
{
	// 클릭을 하면 버튼을 활성화 한다.
	m_JoinButton->SetIsEnabled( false );

	if ( m_MultiPlayerSessionSubsystem )
	{
		m_MultiPlayerSessionSubsystem->FindSessions( 10000 );
	}

	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString( TEXT( "Join Button Clicked" ) ) );
	}
}

////////////////////////////////////////////////////////////////////////////
/// 메뉴 키조작을 합니다.
////////////////////////////////////////////////////////////////////////////
void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* world = GetWorld();
	if ( world )
	{
		APlayerController* playerController = world->GetFirstPlayerController();
		if ( playerController )
		{
			FInputModeGameOnly inputModeData;
			playerController->SetInputMode( inputModeData );
			playerController->SetShowMouseCursor( false );
		}
	}
}
