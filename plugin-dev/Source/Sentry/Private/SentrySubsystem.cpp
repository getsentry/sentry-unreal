// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentryBreadcrumb.h"
#include "SentryEvent.h"
#include "SentryId.h"
#include "SentryUserFeedback.h"

#include "Engine/World.h"
#include "Misc/EngineVersion.h"
#include "Misc/CoreDelegates.h"

#if PLATFORM_ANDROID
#include "Android/SentrySubsystemAndroid.h"
#endif

#if PLATFORM_IOS
#include "IOS/SentrySubsystemIOS.h"
#endif

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
#include "Desktop/SentrySubsystemDesktop.h"
#endif

void USentrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if PLATFORM_ANDROID
	SubsystemNativeImpl = MakeShareable(new SentrySubsystemAndroid());
#elif PLATFORM_IOS
	SubsystemNativeImpl = MakeShareable(new SentrySubsystemIOS());
#elif PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	SubsystemNativeImpl = MakeShareable(new SentrySubsystemDesktop());
#endif

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	if (Settings->InitAutomatically)
	{
		Initialize();
	}
}

void USentrySubsystem::Deinitialize()
{
	Super::Deinitialize();

	DismissAutomaticBreadcrumbs();
}

void USentrySubsystem::Initialize()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->InitWithSettings(Settings);

	AddDefaultContext();
	ConfigureBreadcrumbs();
}

void USentrySubsystem::InitializeWithSettings(const FConfigureSettingsDelegate& OnConfigureSettings)
{
	USentrySettings* Settings = FSentryModule::Get().GetSettings();

	OnConfigureSettings.ExecuteIfBound(Settings);

	Initialize();
}

void USentrySubsystem::Close()
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->Close();
}

void USentrySubsystem::AddBreadcrumb(USentryBreadcrumb* Breadcrumb)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->AddBreadcrumb(Breadcrumb);
}

void USentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	USentryBreadcrumb* Breadcrumb = NewObject<USentryBreadcrumb>();
	Breadcrumb->SetMessage(Message);
	Breadcrumb->SetCategory(Category);
	Breadcrumb->SetType(Type);
	Breadcrumb->SetData(Data);
	Breadcrumb->SetLevel(Level);

	AddBreadcrumb(Breadcrumb);
}

void USentrySubsystem::ClearBreadcrumbs()
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->ClearBreadcrumbs();
}

USentryId* USentrySubsystem::CaptureMessage(const FString& Message, ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return nullptr;

	return SubsystemNativeImpl->CaptureMessage(Message, Level);
}

USentryId* USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return nullptr;

	return SubsystemNativeImpl->CaptureMessageWithScope(Message, OnConfigureScope, Level);
}

USentryId* USentrySubsystem::CaptureEvent(USentryEvent* Event)
{
	if (!SubsystemNativeImpl)
		return nullptr;

	return SubsystemNativeImpl->CaptureEvent(Event);
}

USentryId* USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope)
{
	if (!SubsystemNativeImpl)
		return nullptr;

	return SubsystemNativeImpl->CaptureEventWithScope(Event, OnConfigureScope);
}

void USentrySubsystem::CaptureUserFeedback(USentryUserFeedback* UserFeedback)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->CaptureUserFeedback(UserFeedback);
}

void USentrySubsystem::CaptureUserFeedbackWithParams(USentryId* EventId, const FString& Email, const FString& Comments, const FString& Name)
{
	USentryUserFeedback* UserFeedback = NewObject<USentryUserFeedback>();
	UserFeedback->Initialize(EventId);
	UserFeedback->SetEmail(Email);
	UserFeedback->SetComment(Comments);
	UserFeedback->SetName(Name);

	CaptureUserFeedback(UserFeedback);
}

void USentrySubsystem::SetUser(USentryUser* User)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->SetUser(User);
}

void USentrySubsystem::RemoveUser()
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->RemoveUser();
}

void USentrySubsystem::ConfigureScope(const FConfigureScopeDelegate& OnConfigureScope)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->ConfigureScope(OnConfigureScope);
}

void USentrySubsystem::SetContext(const FString& Key, const TMap<FString, FString>& Values)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->SetContext(Key, Values);
}

void USentrySubsystem::SetTag(const FString& Key, const FString& Value)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->SetTag(Key, Value);
}

void USentrySubsystem::RemoveTag(const FString& Key)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->RemoveTag(Key);
}

void USentrySubsystem::SetLevel(ESentryLevel Level)
{
	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->SetLevel(Level);
}

void USentrySubsystem::AddDefaultContext()
{
	if (!SubsystemNativeImpl)
		return;

	TMap<FString, FString> DefaultContext;
	DefaultContext.Add(TEXT("Engine version"), FEngineVersion::Current().ToString(EVersionComponent::Changelist));
	DefaultContext.Add(TEXT("Plugin version"), FSentryModule::Get().GetPluginVersion());

	SubsystemNativeImpl->SetContext(TEXT("Unreal Engine"), DefaultContext);
}

void USentrySubsystem::ConfigureBreadcrumbs()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if(Settings->AutomaticBreadcrumbs.bOnMapLoadingStarted)
	{
		PreLoadMapDelegate = FCoreUObjectDelegates::PreLoadMap.AddLambda([=](const FString& MapName)
		{
			AddBreadcrumbWithParams(TEXT("PreLoadMap"), TEXT("Unreal"), TEXT("Default"),
				{{TEXT("Map"), MapName}}, ESentryLevel::Info);
		});
	}

	if(Settings->AutomaticBreadcrumbs.bOnMapLoaded)
	{
		PostLoadMapDelegate = FCoreUObjectDelegates::PostLoadMapWithWorld.AddLambda([=](UWorld* World)
		{
			if (World)
			{
				AddBreadcrumbWithParams(TEXT("PostLoadMapWithWorld"), TEXT("Unreal"), TEXT("Default"),
					{{TEXT("Map"), World->GetMapName()}}, ESentryLevel::Info);
			}
			else
			{
				AddBreadcrumbWithParams(TEXT("PostLoadMapWithWorld"), TEXT("Unreal"), TEXT("Default"),
					{{TEXT("Error"), TEXT("Map load failed")}}, ESentryLevel::Error);
			}
		});
	}

	if(Settings->AutomaticBreadcrumbs.bOnGameStateClassChanged)
	{
		GameStateChangedDelegate = FCoreDelegates::GameStateClassChanged.AddLambda([this](const FString& GameState)
		{
			AddBreadcrumbWithParams(TEXT("GameStateClassChanged"), TEXT("Unreal"), TEXT("Default"),
				{{TEXT("GameState"), GameState}}, ESentryLevel::Info);
		});
	}

	if(Settings->AutomaticBreadcrumbs.bOnUserActivityStringChanged)
	{
		UserActivityChangedDelegate = FCoreDelegates::UserActivityStringChanged.AddLambda([this](const FString& Activity)
		{
			AddBreadcrumbWithParams(TEXT("UserActivityStringChanged"), TEXT("Unreal"), TEXT("Default"),
				{{TEXT("Activity"), Activity}}, ESentryLevel::Info);
		});
	}

	if(Settings->AutomaticBreadcrumbs.bOnGameSessionIDChanged)
	{
		GameSessionIDChangedDelegate = FCoreDelegates::GameSessionIDChanged.AddLambda([this](const FString& SessionId)
		{
			AddBreadcrumbWithParams(TEXT("GameSessionIDChanged"), TEXT("Unreal"), TEXT("Default"),
				{{TEXT("Session ID"), SessionId}}, ESentryLevel::Info);
		});
	}
}

void USentrySubsystem::DismissAutomaticBreadcrumbs()
{
	if(PreLoadMapDelegate.IsValid())
	{
		FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapDelegate);
	}

	if(PostLoadMapDelegate.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapDelegate);
	}

	if(GameStateChangedDelegate.IsValid())
	{
		FCoreDelegates::GameStateClassChanged.Remove(GameStateChangedDelegate);
	}

	if(UserActivityChangedDelegate.IsValid())
	{
		FCoreDelegates::UserActivityStringChanged.Remove(UserActivityChangedDelegate);
	}

	if(GameSessionIDChangedDelegate.IsValid())
	{
		FCoreDelegates::GameSessionIDChanged.Remove(GameSessionIDChangedDelegate);
	}
}
