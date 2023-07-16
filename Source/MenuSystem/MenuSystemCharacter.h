// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MenuSystemCharacter.generated.h"


UCLASS(config = Game)
class AMenuSystemCharacter : public ACharacter
{
	GENERATED_BODY()

		/** Camera boom positioning the camera behind the character */
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ))
		class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ))
		class UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = ( AllowPrivateAccess = "true" ))
		class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = ( AllowPrivateAccess = "true" ))
		class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = ( AllowPrivateAccess = "true" ))
		class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = ( AllowPrivateAccess = "true" ))
		class UInputAction* LookAction;

private:
	/// OnlineInterface 정의
	//using IOnlineSessionPtr = TSharedPtr< class IOnlineSession, ESPMode::ThreadSafe >;

public:
	AMenuSystemCharacter();


protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const
	{
		return CameraBoom;
	}
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const
	{
		return FollowCamera;
	}

private:
	// 온라인 서브시스템 세션 인터페이스 
	IOnlineSessionPtr m_onlineSessionInterface;

	/// 세션 생성 완료 대리자 클래스
	FOnCreateSessionCompleteDelegate m_CreateSessionCompleteDelegate;

	/// 세션 찾기 완료 대리자 클래스
	FOnFindSessionsCompleteDelegate m_FindSessionsCompleteDelegate;

	/// 세션 찾기 요청 클래스
	TSharedPtr< FOnlineSessionSearch > m_sessionSearch;

	/// 세션 조인 완료 대리자 클래스
	FOnJoinSessionCompleteDelegate m_joinSessionCompleteDelegate;

protected:
	/// 게임 세션을 생성합니다.
	UFUNCTION( BlueprintCallable )
	void CreateGameSession();

	/// 게임에 
	UFUNCTION( BlueprintCallable )
	void JoinGameSession();

protected:
	/// 세션 생성이 완료 되었습니다.
	void OnCreateSessionComplete( FName sessionName, bool bWasSuccessful );

	/// 세션 생성이 찾기가 완료 되었습니다.
	void OnFindSessionsComplete( bool bWasSuccessful );

	/// 세션 조인이 완료 되었습니다.
	void OnJoinSessionComplete( FName sessionName, EOnJoinSessionCompleteResult::Type result );
};
