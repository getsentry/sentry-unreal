// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEngineSubsystem.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryEvent.h"
#include "SentryId.h"
#include "SentryUserFeedback.h"
#include "SentryBeforeSendHandler.h"

#include "Engine/World.h"
#include "Misc/EngineVersion.h"
#include "Misc/CoreDelegates.h"
#include "Misc/App.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#include "Interface/SentrySubsystemInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentrySubsystemAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentrySubsystemApple.h"
#elif PLATFORM_WINDOWS  || PLATFORM_LINUX
#include "Desktop/SentrySubsystemDesktop.h"
#endif

void USentryEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if PLATFORM_ANDROID
	SubsystemNativeImpl = MakeShareable(new SentrySubsystemAndroid());
#elif PLATFORM_IOS || PLATFORM_MAC
	SubsystemNativeImpl = MakeShareable(new SentrySubsystemApple());
#elif (PLATFORM_WINDOWS || PLATFORM_LINUX) && USE_SENTRY_NATIVE
	SubsystemNativeImpl = MakeShareable(new SentrySubsystemDesktop());
#endif

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	UE_LOG(LogSentrySdk, Log, TEXT("Sentry plugin auto initialization: %s"), Settings->InitAutomatically ? TEXT("true") : TEXT("false"));

	if (Settings->InitAutomatically)
	{
		if(!GIsEditor || Settings->CrashCapturingMode == ESentryCrashCapturingMode::GameAndEditor)
		{
			Initialize();
		}
	}
}

void USentryEngineSubsystem::Deinitialize()
{
	Close();

	Super::Deinitialize();
}

void USentryEngineSubsystem::Initialize()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if(Settings->Dsn.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry requires minimal configuration for its initialization - please provide the DSN in plugin settings."));
		return;
	}

	if(!IsCurrentBuildConfigurationEnabled() || !IsCurrentBuildTargetEnabled() || !IsCurrentPlatformEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry initialization skipped since event capturing is disabled for the current configuration/target/platform/build in plugin settings."));
		return;
	}

	if(Settings->EnableForPromotedBuildsOnly && !FApp::GetEngineIsPromotedBuild())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry initialization skipped since event capturing is disabled for the non-promoted builds in plugin settings."));
		return;
	}

	const UClass* BeforeSendHandlerClass = Settings->BeforeSendHandler != nullptr
		? static_cast<UClass*>(Settings->BeforeSendHandler)
		: USentryBeforeSendHandler::StaticClass();

	BeforeSendHandler = NewObject<USentryBeforeSendHandler>(this, BeforeSendHandlerClass);

	if (!SubsystemNativeImpl)
		return;

	SubsystemNativeImpl->InitWithSettings(Settings, BeforeSendHandler);

	if(!SubsystemNativeImpl->IsEnabled())
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Sentry initialization failed."));
		return;
	}

	AddDefaultContext();

#if PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC
	AddGpuContext();
	AddDeviceContext();
#endif

	PromoteTags();
	ConfigureBreadcrumbs();
}

void USentryEngineSubsystem::InitializeWithSettings(const FConfigureSettingsDelegate& OnConfigureSettings)
{
	USentrySettings* Settings = FSentryModule::Get().GetSettings();

	OnConfigureSettings.ExecuteIfBound(Settings);

	Initialize();
}

void USentryEngineSubsystem::Close()
{
	DisableAutomaticBreadcrumbs();

	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->Close();
}

bool USentryEngineSubsystem::IsEnabled()
{
	if (!SubsystemNativeImpl)
		return false;

	return SubsystemNativeImpl->IsEnabled();
}

void USentryEngineSubsystem::AddBreadcrumb(USentryBreadcrumb* Breadcrumb)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->AddBreadcrumb(Breadcrumb);
}

void USentryEngineSubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	USentryBreadcrumb* Breadcrumb = NewObject<USentryBreadcrumb>();
	Breadcrumb->SetMessage(Message);
	Breadcrumb->SetCategory(Category);
	Breadcrumb->SetType(Type);
	Breadcrumb->SetData(Data);
	Breadcrumb->SetLevel(Level);

	AddBreadcrumb(Breadcrumb);
}

void USentryEngineSubsystem::ClearBreadcrumbs()
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->ClearBreadcrumbs();
}

USentryId* USentryEngineSubsystem::CaptureMessage(const FString& Message, ESentryLevel Level)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return nullptr;
	}

	return SubsystemNativeImpl->CaptureMessage(Message, Level);
}

USentryId* USentryEngineSubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return nullptr;
	}

	return SubsystemNativeImpl->CaptureMessageWithScope(Message, OnConfigureScope, Level);
}

USentryId* USentryEngineSubsystem::CaptureEvent(USentryEvent* Event)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return nullptr;
	}

	return SubsystemNativeImpl->CaptureEvent(Event);
}

USentryId* USentryEngineSubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return nullptr;
	}

	return SubsystemNativeImpl->CaptureEventWithScope(Event, OnConfigureScope);
}

void USentryEngineSubsystem::CaptureUserFeedback(USentryUserFeedback* UserFeedback)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->CaptureUserFeedback(UserFeedback);
}

void USentryEngineSubsystem::CaptureUserFeedbackWithParams(USentryId* EventId, const FString& Email, const FString& Comments, const FString& Name)
{
	USentryUserFeedback* UserFeedback = NewObject<USentryUserFeedback>();
	UserFeedback->Initialize(EventId);
	UserFeedback->SetEmail(Email);
	UserFeedback->SetComment(Comments);
	UserFeedback->SetName(Name);

	CaptureUserFeedback(UserFeedback);
}

void USentryEngineSubsystem::SetUser(USentryUser* User)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->SetUser(User);
}

void USentryEngineSubsystem::RemoveUser()
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->RemoveUser();
}

void USentryEngineSubsystem::ConfigureScope(const FConfigureScopeDelegate& OnConfigureScope)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->ConfigureScope(OnConfigureScope);
}

void USentryEngineSubsystem::SetContext(const FString& Key, const TMap<FString, FString>& Values)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->SetContext(Key, Values);
}

void USentryEngineSubsystem::SetTag(const FString& Key, const FString& Value)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->SetTag(Key, Value);
}

void USentryEngineSubsystem::RemoveTag(const FString& Key)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->RemoveTag(Key);
}

void USentryEngineSubsystem::SetLevel(ESentryLevel Level)
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->SetLevel(Level);
}

void USentryEngineSubsystem::StartSession()
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->StartSession();
}

void USentryEngineSubsystem::EndSession()
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	SubsystemNativeImpl->EndSession();
}

void USentryEngineSubsystem::AddDefaultContext()
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	TMap<FString, FString> DefaultContext;
	DefaultContext.Add(TEXT("Engine version"), FEngineVersion::Current().ToString(EVersionComponent::Changelist));
	DefaultContext.Add(TEXT("Plugin version"), FSentryModule::Get().GetPluginVersion());
	DefaultContext.Add(TEXT("Configuration"), LexToString(FApp::GetBuildConfiguration()));
	DefaultContext.Add(TEXT("Target Type"), LexToString(FApp::GetBuildTargetType()));
	DefaultContext.Add(TEXT("Engine mode"), FGenericPlatformMisc::GetEngineMode());
	DefaultContext.Add(TEXT("Is game"), FApp::IsGame() ? TEXT("True") : TEXT("False"));
	DefaultContext.Add(TEXT("Is standalone"), FApp::IsStandalone() ? TEXT("True") : TEXT("False"));
	DefaultContext.Add(TEXT("Is unattended"), FApp::IsUnattended() ? TEXT("True") : TEXT("False"));
	DefaultContext.Add(TEXT("Game name"), FApp::GetName());

	SubsystemNativeImpl->SetContext(TEXT("Unreal Engine"), DefaultContext);
}

void USentryEngineSubsystem::AddGpuContext()
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	FGPUDriverInfo GpuDriverInfo = FPlatformMisc::GetGPUDriverInfo(FPlatformMisc::GetPrimaryGPUBrand());

	TMap<FString, FString> GpuContext;
	GpuContext.Add(TEXT("name"), GpuDriverInfo.DeviceDescription);
	GpuContext.Add(TEXT("vendor_name"), GpuDriverInfo.ProviderName);
	GpuContext.Add(TEXT("driver_version"), GpuDriverInfo.UserDriverVersion);

	SubsystemNativeImpl->SetContext(TEXT("gpu"), GpuContext);
}

void USentryEngineSubsystem::AddDeviceContext()
{
	if (!IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is not initialized."));
		return;
	}

	const FPlatformMemoryConstants& MemoryConstants = FPlatformMemory::GetConstants();

	TMap<FString, FString> DeviceContext;
	DeviceContext.Add(TEXT("cpu_description"), FPlatformMisc::GetCPUBrand());
	DeviceContext.Add(TEXT("number_of_cores"), FString::FromInt(FPlatformMisc::NumberOfCores()));
	DeviceContext.Add(TEXT("number_of_cores_including_hyperthreads"), FString::FromInt(FPlatformMisc::NumberOfCoresIncludingHyperthreads()));
	DeviceContext.Add(TEXT("physical_memory_size_gb"), FString::FromInt(MemoryConstants.TotalPhysicalGB));

	SubsystemNativeImpl->SetContext(TEXT("device"), DeviceContext);
}

void USentryEngineSubsystem::PromoteTags()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if(Settings->TagsPromotion.bPromoteBuildConfiguration)
		SubsystemNativeImpl->SetTag(TEXT("Configuration"), LexToString(FApp::GetBuildConfiguration()));
	if(Settings->TagsPromotion.bPromoteTargetType)
		SubsystemNativeImpl->SetTag(TEXT("Target Type"), LexToString(FApp::GetBuildTargetType()));
	if(Settings->TagsPromotion.bPromoteEngineMode)
		SubsystemNativeImpl->SetTag(TEXT("Engine Mode"), FGenericPlatformMisc::GetEngineMode());
	if(Settings->TagsPromotion.bPromoteIsGame)
		SubsystemNativeImpl->SetTag(TEXT("Is game"), FApp::IsGame() ? TEXT("True") : TEXT("False"));
	if(Settings->TagsPromotion.bPromoteIsStandalone)
		SubsystemNativeImpl->SetTag(TEXT("Is standalone"), FApp::IsStandalone() ? TEXT("True") : TEXT("False"));
	if(Settings->TagsPromotion.bPromoteIsUnattended)
		SubsystemNativeImpl->SetTag(TEXT("Is unattended"), FApp::IsUnattended() ? TEXT("True") : TEXT("False"));
}

void USentryEngineSubsystem::ConfigureBreadcrumbs()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	if(Settings->AutomaticBreadcrumbs.bOnMapLoadingStarted)
	{
		PreLoadMapDelegate = FCoreUObjectDelegates::PreLoadMap.AddLambda([this](const FString& MapName)
		{
			AddBreadcrumbWithParams(TEXT("PreLoadMap"), TEXT("Unreal"), TEXT("Default"),
				{{TEXT("Map"), MapName}}, ESentryLevel::Info);
		});
	}

	if(Settings->AutomaticBreadcrumbs.bOnMapLoaded)
	{
		PostLoadMapDelegate = FCoreUObjectDelegates::PostLoadMapWithWorld.AddLambda([this](UWorld* World)
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

void USentryEngineSubsystem::DisableAutomaticBreadcrumbs()
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

bool USentryEngineSubsystem::IsCurrentBuildConfigurationEnabled()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	bool IsBuildConfigurationEnabled;

	switch (FApp::GetBuildConfiguration())
	{
	case EBuildConfiguration::Debug:
		IsBuildConfigurationEnabled = Settings->EnableBuildConfigurations.bEnableDebug;
		break;
	case EBuildConfiguration::DebugGame:
		IsBuildConfigurationEnabled = Settings->EnableBuildConfigurations.bEnableDebugGame;
		break;
	case EBuildConfiguration::Development:
		IsBuildConfigurationEnabled = Settings->EnableBuildConfigurations.bEnableDevelopment;
		break;
	case EBuildConfiguration::Shipping:
		IsBuildConfigurationEnabled = Settings->EnableBuildConfigurations.bEnableShipping;
		break;
	case EBuildConfiguration::Test:
		IsBuildConfigurationEnabled = Settings->EnableBuildConfigurations.bEnableTest;
		break;
	default:
		IsBuildConfigurationEnabled = false;
	}

	return IsBuildConfigurationEnabled;
}

bool USentryEngineSubsystem::IsCurrentBuildTargetEnabled()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	bool IsBuildTargetTypeEnabled;

	switch (FApp::GetBuildTargetType())
	{
	case EBuildTargetType::Game:
		IsBuildTargetTypeEnabled = Settings->EnableBuildTargets.bEnableGame;
		break;
	case EBuildTargetType::Server:
		IsBuildTargetTypeEnabled = Settings->EnableBuildTargets.bEnableServer;
		break;
	case EBuildTargetType::Client:
		IsBuildTargetTypeEnabled = Settings->EnableBuildTargets.bEnableClient;
		break;
	case EBuildTargetType::Editor:
		IsBuildTargetTypeEnabled = Settings->EnableBuildTargets.bEnableEditor;
		break;
	case EBuildTargetType::Program:
		IsBuildTargetTypeEnabled = Settings->EnableBuildTargets.bEnableProgram;
		break;
	default:
		IsBuildTargetTypeEnabled = false;
	}

	return IsBuildTargetTypeEnabled;
}

bool USentryEngineSubsystem::IsCurrentPlatformEnabled()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	bool IsBuildPlatformEnabled = false;

#if PLATFORM_LINUX
	IsBuildPlatformEnabled = Settings->EnableBuildPlatforms.bEnableLinux;
#elif PLATFORM_IOS
	IsBuildPlatformEnabled = Settings->EnableBuildPlatforms.bEnableIOS;
#elif PLATFORM_WINDOWS
	IsBuildPlatformEnabled = Settings->EnableBuildPlatforms.bEnableWindows;
#elif PLATFORM_ANDROID
	IsBuildPlatformEnabled = Settings->EnableBuildPlatforms.bEnableAndroid;
#elif PLATFORM_MAC
	IsBuildPlatformEnabled = Settings->EnableBuildPlatforms.bEnableMac;
#endif

	return IsBuildPlatformEnabled;
}