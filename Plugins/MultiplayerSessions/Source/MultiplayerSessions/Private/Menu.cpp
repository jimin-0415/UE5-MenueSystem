// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"


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

}
