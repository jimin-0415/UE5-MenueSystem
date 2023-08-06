// Copyright Epic Games, Inc. All Rights Reserved.

#include "MenuSystemCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"


//////////////////////////////////////////////////////////////////////////
// AMenuSystemCharacter

AMenuSystemCharacter::AMenuSystemCharacter()
: 
// Binding Callback
m_CreateSessionCompleteDelegate( 
	FOnCreateSessionCompleteDelegate::CreateUObject( this, &AMenuSystemCharacter::OnCreateSessionComplete ) ),
m_FindSessionsCompleteDelegate( 
	FOnFindSessionsCompleteDelegate::CreateUObject( this, &AMenuSystemCharacter::OnFindSessionsComplete ) ),
m_joinSessionCompleteDelegate(
	FOnJoinSessionCompleteDelegate::CreateUObject( this, &AMenuSystemCharacter::OnJoinSessionComplete ) )
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// 인터페이스로부터 OnlineSubSystem 정보를 가져온다.
	// 온라인 서브 시스템은 기본 Unreal Editor로 실행시에는 연결을 확인할 수 없다. 
	// Package 파일로 뽑아서 접속을 하면, 연결을 확인할 수 있다.
	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	if ( onlineSubsystem )
	{
		m_onlineSessionInterface = onlineSubsystem->GetSessionInterface();

		/*if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( 
				-1,
				15.f, 
				FColor::Blue, 
				FString::Printf( TEXT("Found subsystem %s"), *onlineSubsystem->GetSubsystemName().ToString() )
			);
		}*/
	}
}

void AMenuSystemCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMenuSystemCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMenuSystemCharacter::Look);

	}

}

void AMenuSystemCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMenuSystemCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

//////////////////////////////////////////////////////////////////////////
// 게임 세션을 생성합니다.
//////////////////////////////////////////////////////////////////////////
void AMenuSystemCharacter::CreateGameSession()
{
	// Called When Press 1 Key
	// Interface 유효성 검증
	if ( !m_onlineSessionInterface.IsValid() )
		return;
	
	// 기존 세션 존재 여부 확인
	auto existingSession = m_onlineSessionInterface->GetNamedSession( NAME_GameSession );
	if ( nullptr != existingSession )
	{
		m_onlineSessionInterface->DestroySession( NAME_GameSession );
	}

	// 서브시스템 세션에 '델리게이트 세션 생성 완료 델리게이터' 등록
	m_onlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle( m_CreateSessionCompleteDelegate );

	// 세션 설정을 시작합니다. 
	TSharedPtr<FOnlineSessionSettings> sessionSettings = MakeShareable( new FOnlineSessionSettings() );
	sessionSettings->bIsLANMatch			= false;	
	sessionSettings->NumPublicConnections	= 4;	//몇명의 Player랑 할거냐
	sessionSettings->bAllowJoinInProgress	= true; //실행 도중 다른 유저의 난입이 가능하냐
	sessionSettings->bAllowJoinViaPresence	= true; 
	sessionSettings->bShouldAdvertise		= true;
	sessionSettings->bUsesPresence			= true;
	sessionSettings->bUseLobbiesIfAvailable = true;
	sessionSettings->Set( FName( "MatchType" ), FString( "FreeForAll" ), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing );

	// 월드로부터 로컬플레이어 정보를 가져온다. 각 로컬 플레이어는 고유의 Id값을 가진다.
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	// 세션 생성
	m_onlineSessionInterface->CreateSession( *localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *sessionSettings );
}

//////////////////////////////////////////////////////////////////////////
// 세션 생성이 완료 되었습니다.
//////////////////////////////////////////////////////////////////////////
void AMenuSystemCharacter::JoinGameSession()
{
	//Find Game Session.
	if ( !m_onlineSessionInterface.IsValid() )
		return;

	// 서브시스템 세션에 '델리게이트 세션 찾기 완료 델리게이터' 등록
	m_onlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle( m_FindSessionsCompleteDelegate );

	m_sessionSearch = MakeShareable( new FOnlineSessionSearch() );
	m_sessionSearch->MaxSearchResults				= 10000;	// 80, 480 은 Steam에서 제공해주는 Session 넘버
	m_sessionSearch->bIsLanQuery					= false;

	m_sessionSearch->QuerySettings.Set( SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals ); // 세션 검색 쿼리 세팅 

	// 월드로부터 로컬플레이어 정보를 가져온다. 각 로컬 플레이어는 고유의 Id값을 가진다.
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	// 세션 찾기
	m_onlineSessionInterface->FindSessions( *localPlayer->GetPreferredUniqueNetId(), m_sessionSearch.ToSharedRef() );
}

//////////////////////////////////////////////////////////////////////////
// 세션 생성이 완료 되었습니다.
//////////////////////////////////////////////////////////////////////////
void AMenuSystemCharacter::OnCreateSessionComplete(FName sessionName, bool bWasSuccessful)
{
	if ( bWasSuccessful )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf( TEXT( "Surccess to Create Session : %s" ), *sessionName.ToString() )
			);
		}

		UWorld* world = GetWorld();
		if ( world )
		{
			// ServerTravel 를 호출해서 로비 레벨로 이동하고 listen 서버 오픈
			FString levelPath = FString("/Game/ThirdPerson/Maps/Lobby?listen");
			world->ServerTravel( levelPath );
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
				FString( TEXT( "Failed to Create Session!" ) )
			);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// 세션 생성이 찾기가 완료 되었습니다.
//////////////////////////////////////////////////////////////////////////
void AMenuSystemCharacter::OnFindSessionsComplete( bool bWasSuccessful )
{
	if ( !m_onlineSessionInterface.IsValid() )
		return;

	// 검색한 Session의 결과 정보를 가져온다.
	for ( auto& result : m_sessionSearch->SearchResults )
	{
		//(FString)
		auto id = result.GetSessionIdStr();
		auto user = result.Session.OwningUserName;
		FString matchType;

		result.Session.SessionSettings.Get( FName( "MatchType" ), matchType );

		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Cyan,
				FString::Printf( TEXT( "Id: %s, User: %s" ), *id, *user )
			);
		}

		if ( matchType == FString( "FreeForAll" ) )
		{
			if ( GEngine )
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Cyan,
					FString::Printf( TEXT( "Joining Match Type: %s" ), *matchType )
				);
			}
			
			// Join 완료시 알려준다.
			m_onlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle( m_joinSessionCompleteDelegate );
			
			const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			m_onlineSessionInterface->JoinSession( *localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, result );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// 세션 조인이 완료 되었습니다.
//////////////////////////////////////////////////////////////////////////
void AMenuSystemCharacter::OnJoinSessionComplete( FName sessionName, EOnJoinSessionCompleteResult::Type result )
{
	if ( !m_onlineSessionInterface.IsValid() )
		return;
	
	FString address;
	if ( m_onlineSessionInterface->GetResolvedConnectString( NAME_GameSession, address ) )
	{
		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString::Printf( TEXT( "Connect String: %s" ), *address )
			);
		}

		APlayerController* playerController = GetGameInstance()->GetFirstLocalPlayerController();
		if ( playerController )
		{
			playerController->ClientTravel( address, ETravelType::TRAVEL_Absolute );
		}
	}
}
