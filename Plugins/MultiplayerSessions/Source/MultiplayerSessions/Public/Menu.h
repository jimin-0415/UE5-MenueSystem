// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
public:
	/// 메뉴 초기 설정을 합니다.
	UFUNCTION( BlueprintCallable )
	void MenuSetup();

protected:
	/// 초기화를 진행합니다.
	virtual bool Initialize() override;

private:
	/// 바인딩되는 버튼의 이름은 블루프린트에 정의된 이름과 동일해야 한다.
	// UPROPERTY( meta = ( BindWidget ) ) 

	/// Host 버튼
	UPROPERTY( meta = ( BindWidget ) )
	UButton* m_HostButton;

	/// Join 버튼
	UPROPERTY( meta = ( BindWidget ) )
	UButton* m_JoinButton;

private:
	/// Host 버튼을 클릭합니다.
	UFUNCTION()
	void HostButtonClicked();

	/// Join 버튼을 클릭합니다.
	UFUNCTION()
	void JoinButtonClicked();

private:
	/// The subsystem designed to handl all online session functionality
	UMultiPlayerSessionsSubsystem* m_MultiPlayerSessionSubsystem;
};
