// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiPlayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"

////////////////////////////////////////////////////////////////////////////
/// 생성자
////////////////////////////////////////////////////////////////////////////
UMultiPlayerSessionsSubsystem::UMultiPlayerSessionsSubsystem()
{
	IOnlineSubsystem* subSystem = IOnlineSubsystem::Get();
	if ( subSystem )
	{
		// 서브 시스템으로 부터 세션 관리가 가능한 세션 인터페이스 정보를 가져온다.
		m_sessionInterface = subSystem->GetSessionInterface();
	}
}
