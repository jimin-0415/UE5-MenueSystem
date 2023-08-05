// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiPlayerSessionsSubsystem.h"
#include "Components/Button.h"


////////////////////////////////////////////////////////////////////////////
/// 메뉴 초기 설정을 합니다.
////////////////////////////////////////////////////////////////////////////
void UMenu::MenuSetup( int32 numberOfPublicConnections, FString typeOfMatch )
{
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
		/// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam 에 바인딩 되는 함수는 UFUNC() 매크로 선언이 필요하다.
		m_MultiPlayerSessionSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic( this, &ThisClass::OnCreateSession );
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

void UMenu::NativeDestruct()
{
	MenuTearDown();

	Super::NativeDestruct();
}

////////////////////////////////////////////////////////////////////////////
/// 세션이 생성되었을 때 처리합니다.
////////////////////////////////////////////////////////////////////////////
void UMenu::OnCreateSession( bool bWasSuccessful )
{
	if ( bWasSuccessful )
	{
		// 정상적으로 Session이 만들어졌다고 판단되면 World 이동
		UWorld* world = GetWorld();
		if ( world )
		{
			world->ServerTravel( "/Game/ThirdPerson/Maps/Lobby?listen" );
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
	}
}

////////////////////////////////////////////////////////////////////////////
/// Host 버튼을 클릭합니다.
////////////////////////////////////////////////////////////////////////////
void UMenu::HostButtonClicked()
{
	if ( m_MultiPlayerSessionSubsystem )
	{
		/// TODO. 일단 들어오는지 검사하기 위해서 임시로 적용.
		m_MultiPlayerSessionSubsystem->CreateSession( m_NumPublicConnections, m_MatchType );
	}
}

////////////////////////////////////////////////////////////////////////////
/// Join 버튼을 클릭합니다..
////////////////////////////////////////////////////////////////////////////
void UMenu::JoinButtonClicked()
{
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
