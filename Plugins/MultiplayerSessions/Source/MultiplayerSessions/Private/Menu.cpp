// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "MultiPlayerSessionsSubsystem.h"
#include "Components/Button.h"


////////////////////////////////////////////////////////////////////////////
/// 메뉴 초기 설정을 합니다.
////////////////////////////////////////////////////////////////////////////
void UMenu::MenuSetup()
{
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
/// Host 버튼을 클릭합니다.
////////////////////////////////////////////////////////////////////////////
void UMenu::HostButtonClicked()
{
	if ( GEngine )
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString( TEXT( "Host Button Clicked" ) ) );
	}

	if ( m_MultiPlayerSessionSubsystem )
	{
		/// TODO. 일단 들어오는지 검사하기 위해서 임시로 적용.
		m_MultiPlayerSessionSubsystem->CreateSession( 4, FString( "FreForAll" ) );
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
