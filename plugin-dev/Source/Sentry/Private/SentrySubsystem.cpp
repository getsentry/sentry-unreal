// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"

#include "SentryModule.h"
#include "SentrySettings.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryEvent.h"
#include "SentryId.h"
#include "SentryUser.h"
#include "SentryUserFeedback.h"
#include "SentryBeforeSendHandler.h"
#include "SentryTraceSampler.h"
#include "SentryTransaction.h"
#include "SentryTransactionContext.h"
#include "SentryOutputDevice.h"
#include "SentryOutputDeviceError.h"

#include "CoreGlobals.h"
#include "Engine/World.h"
#include "Misc/EngineVersion.h"
#include "Misc/CoreDelegates.h"
#include "Misc/App.h"
#include "Misc/AssertionMacros.h"
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

void USentrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
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
		Initialize();
	}
}

void USentrySubsystem::Deinitialize()
{
	DisableAutomaticBreadcrumbs();

	Close();

	Super::Deinitialize();
}

void USentrySubsystem::Initialize()
{
	if (!SubsystemNativeImpl)
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry subsystem is invalid and can't be initialized."));
		return;
	}

	if (SubsystemNativeImpl->IsEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry is already initialized. It will be shut down automatically before re-init."));
		Close();
	}

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

	if(IsPromotedBuildsOnlyEnabled() && !FApp::GetEngineIsPromotedBuild())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry initialization skipped since event capturing is disabled for the non-promoted builds in plugin settings."));
		return;
	}

	const UClass* BeforeSendHandlerClass = Settings->BeforeSendHandler != nullptr
		? static_cast<UClass*>(Settings->BeforeSendHandler)
		: USentryBeforeSendHandler::StaticClass();

	BeforeSendHandler = NewObject<USentryBeforeSendHandler>(this, BeforeSendHandlerClass);

	const UClass* TraceSamplerClass = Settings->TracesSampler != nullptr
		? static_cast<UClass*>(Settings->TracesSampler)
		: USentryTraceSampler::StaticClass();

	TraceSampler = NewObject<USentryTraceSampler>(this, TraceSamplerClass);

	SubsystemNativeImpl->InitWithSettings(Settings, BeforeSendHandler, TraceSampler);

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

	ConfigureOutputDevice();

#if PLATFORM_WINDOWS
	if (FEngineVersion::Current().GetMajor() == 5 && FEngineVersion::Current().GetMinor() >= 2)
	{
		if (Settings->EnableAutoCrashCapturing)
		{
			ConfigureOutputDeviceError();
		}
	}
#else
	ConfigureOutputDeviceError();
#endif

	OnEnsureDelegate = FCoreDelegates::OnHandleSystemEnsure.AddLambda([this]()
	{
		FString EnsureMessage = GErrorHist;
		SubsystemNativeImpl->CaptureEnsure(TEXT("Ensure failed"), EnsureMessage.TrimStartAndEnd());
	});
}

void USentrySubsystem::InitializeWithSettings(const FConfigureSettingsDelegate& OnConfigureSettings)
{
	USentrySettings* Settings = FSentryModule::Get().GetSettings();

	OnConfigureSettings.ExecuteIfBound(Settings);

	Initialize();
}

void USentrySubsystem::Close()
{
	if(GLog && OutputDevice)
	{
		GLog->RemoveOutputDevice(OutputDevice.Get());
		OutputDevice = nullptr;
	}

	if(GError && OutputDeviceError)
	{
		if(OnAssertDelegate.IsValid())
		{
			OutputDeviceError->OnAssert.Remove(OnAssertDelegate);
			OnAssertDelegate.Reset();
		}

		GError = OutputDeviceError->GetParentDevice();
		OutputDeviceError = nullptr;
	}

	if(OnEnsureDelegate.IsValid())
	{
		FCoreDelegates::OnHandleSystemEnsure.Remove(OnEnsureDelegate);
		OnEnsureDelegate.Reset();
	}

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->Close();
}

bool USentrySubsystem::IsEnabled()
{
	if (!SubsystemNativeImpl)
		return false;

	return SubsystemNativeImpl->IsEnabled();
}

ESentryCrashedLastRun USentrySubsystem::IsCrashedLastRun()
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return ESentryCrashedLastRun::NotEvaluated;

	return SubsystemNativeImpl->IsCrashedLastRun();
}

void USentrySubsystem::AddBreadcrumb(USentryBreadcrumb* Breadcrumb)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->AddBreadcrumb(Breadcrumb->GetNativeImpl());
}

void USentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->AddBreadcrumbWithParams(Message, Category, Type, Data, Level);
}

void USentrySubsystem::ClearBreadcrumbs()
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->ClearBreadcrumbs();
}

USentryId* USentrySubsystem::CaptureMessage(const FString& Message, ESentryLevel Level)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return nullptr;

	TSharedPtr<ISentryId> idNativeImpl = SubsystemNativeImpl->CaptureMessage(Message, Level);

	USentryId* unrealId = NewObject<USentryId>();
	unrealId->InitWithNativeImpl(idNativeImpl);

	return unrealId;
}

USentryId* USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level)
{
	return CaptureMessageWithScope(Message, FConfigureScopeNativeDelegate::CreateUFunction(const_cast<UObject*>(OnConfigureScope.GetUObject()), OnConfigureScope.GetFunctionName()), Level);
}

USentryId* USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeNativeDelegate& OnConfigureScope, ESentryLevel Level)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return nullptr;

	TSharedPtr<ISentryId> idNativeImpl = SubsystemNativeImpl->CaptureMessageWithScope(Message, FSentryScopeDelegate::CreateLambda([&](TSharedPtr<ISentryScope> nativeScope)
	{
		USentryScope* unrealScope = NewObject<USentryScope>();
		unrealScope->InitWithNativeImpl(nativeScope);
		OnConfigureScope.ExecuteIfBound(unrealScope);
	}), Level);

	USentryId* unrealId = NewObject<USentryId>();
	unrealId->InitWithNativeImpl(idNativeImpl);

	return unrealId;
}

USentryId* USentrySubsystem::CaptureEvent(USentryEvent* Event)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return nullptr;

	TSharedPtr<ISentryId> idNativeImpl = SubsystemNativeImpl->CaptureEvent(Event->GetNativeImpl());

	USentryId* unrealId = NewObject<USentryId>();
	unrealId->InitWithNativeImpl(idNativeImpl);

	return unrealId;
}

USentryId* USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope)
{
	return CaptureEventWithScope(Event, FConfigureScopeNativeDelegate::CreateUFunction(const_cast<UObject*>(OnConfigureScope.GetUObject()), OnConfigureScope.GetFunctionName()));
}

USentryId* USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeNativeDelegate& OnConfigureScope)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return nullptr;

	TSharedPtr<ISentryId> idNativeImpl = SubsystemNativeImpl->CaptureEventWithScope(Event->GetNativeImpl(), FSentryScopeDelegate::CreateLambda([&](TSharedPtr<ISentryScope> nativeScope)
	{
		USentryScope* unrealScope = NewObject<USentryScope>();
		unrealScope->InitWithNativeImpl(nativeScope);
		OnConfigureScope.ExecuteIfBound(unrealScope);
	}));

	USentryId* unrealId = NewObject<USentryId>();
	unrealId->InitWithNativeImpl(idNativeImpl);

	return unrealId;
}

void USentrySubsystem::CaptureUserFeedback(USentryUserFeedback* UserFeedback)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->CaptureUserFeedback(UserFeedback->GetNativeImpl());
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
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->SetUser(User->GetNativeImpl());
}

void USentrySubsystem::RemoveUser()
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->RemoveUser();
}

void USentrySubsystem::ConfigureScope(const FConfigureScopeDelegate& OnConfigureScope)
{
	ConfigureScope(FConfigureScopeNativeDelegate::CreateUFunction(const_cast<UObject*>(OnConfigureScope.GetUObject()), OnConfigureScope.GetFunctionName()));
}

void USentrySubsystem::ConfigureScope(const FConfigureScopeNativeDelegate& OnConfigureScope)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->ConfigureScope(FSentryScopeDelegate::CreateLambda([&](TSharedPtr<ISentryScope> nativeScope)
	{
		USentryScope* unrealScope = NewObject<USentryScope>();
		unrealScope->InitWithNativeImpl(nativeScope);
		OnConfigureScope.ExecuteIfBound(unrealScope);
	}));
}

void USentrySubsystem::SetContext(const FString& Key, const TMap<FString, FString>& Values)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->SetContext(Key, Values);
}

void USentrySubsystem::SetTag(const FString& Key, const FString& Value)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->SetTag(Key, Value);
}

void USentrySubsystem::RemoveTag(const FString& Key)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->RemoveTag(Key);
}

void USentrySubsystem::SetLevel(ESentryLevel Level)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->SetLevel(Level);
}

void USentrySubsystem::StartSession()
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->StartSession();
}

void USentrySubsystem::EndSession()
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	SubsystemNativeImpl->EndSession();
}

USentryTransaction* USentrySubsystem::StartTransaction(const FString& Name, const FString& Operation)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return nullptr;

	TSharedPtr<ISentryTransaction> transactionNativeImpl = SubsystemNativeImpl->StartTransaction(Name, Operation);

	USentryTransaction* unrealTransaction = NewObject<USentryTransaction>();
	unrealTransaction->InitWithNativeImpl(transactionNativeImpl);

	return unrealTransaction;
}

USentryTransaction* USentrySubsystem::StartTransactionWithContext(USentryTransactionContext* Context)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return nullptr;

	TSharedPtr<ISentryTransaction> transactionNativeImpl = SubsystemNativeImpl->StartTransactionWithContext(Context->GetNativeImpl());

	USentryTransaction* unrealTransaction = NewObject<USentryTransaction>();
	unrealTransaction->InitWithNativeImpl(transactionNativeImpl);

	return unrealTransaction;
}

USentryTransaction* USentrySubsystem::StartTransactionWithContextAndTimestamp(USentryTransactionContext* Context, int64 Timestamp)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return nullptr;

	TSharedPtr<ISentryTransaction> transactionNativeImpl = SubsystemNativeImpl->StartTransactionWithContextAndTimestamp(Context->GetNativeImpl(), Timestamp);

	USentryTransaction* unrealTransaction = NewObject<USentryTransaction>();
	unrealTransaction->InitWithNativeImpl(transactionNativeImpl);

	return unrealTransaction;
}

USentryTransaction* USentrySubsystem::StartTransactionWithContextAndOptions(USentryTransactionContext* Context, const TMap<FString, FString>& Options)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return nullptr;

	TSharedPtr<ISentryTransaction> transactionNativeImpl = SubsystemNativeImpl->StartTransactionWithContextAndOptions(Context->GetNativeImpl(), Options);

	USentryTransaction* unrealTransaction = NewObject<USentryTransaction>();
	unrealTransaction->InitWithNativeImpl(transactionNativeImpl);

	return unrealTransaction;
}

USentryTransactionContext* USentrySubsystem::ContinueTrace(const FString& SentryTrace, const TArray<FString>& BaggageHeaders)
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return nullptr;

	TSharedPtr<ISentryTransactionContext> transactionContextNativeImpl = SubsystemNativeImpl->ContinueTrace(SentryTrace, BaggageHeaders);

	USentryTransactionContext* unrealTransactionContext = NewObject<USentryTransactionContext>();
	unrealTransactionContext->InitWithNativeImpl(transactionContextNativeImpl);

	return unrealTransactionContext;
}

bool USentrySubsystem::IsSupportedForCurrentSettings()
{
	if(!IsCurrentBuildConfigurationEnabled() || !IsCurrentBuildTargetEnabled() || !IsCurrentPlatformEnabled())
	{
		return false;
	}

	if(IsPromotedBuildsOnlyEnabled() && !FApp::GetEngineIsPromotedBuild())
	{
		return false;
	}

	return true;
}

void USentrySubsystem::AddDefaultContext()
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	TMap<FString, FString> DefaultContext;
	DefaultContext.Add(TEXT("Engine version"), FEngineVersion::Current().ToString(EVersionComponent::Changelist));
	DefaultContext.Add(TEXT("Plugin version"), FSentryModule::Get().GetPluginVersion());
	DefaultContext.Add(TEXT("Is Marketplace version"), FSentryModule::Get().IsMarketplaceVersion() ? TEXT("True") : TEXT("False"));
	DefaultContext.Add(TEXT("Configuration"), LexToString(FApp::GetBuildConfiguration()));
	DefaultContext.Add(TEXT("Target Type"), LexToString(FApp::GetBuildTargetType()));
	DefaultContext.Add(TEXT("Engine mode"), FGenericPlatformMisc::GetEngineMode());
	DefaultContext.Add(TEXT("Is game"), FApp::IsGame() ? TEXT("True") : TEXT("False"));
	DefaultContext.Add(TEXT("Is standalone"), FApp::IsStandalone() ? TEXT("True") : TEXT("False"));
	DefaultContext.Add(TEXT("Is unattended"), FApp::IsUnattended() ? TEXT("True") : TEXT("False"));
	DefaultContext.Add(TEXT("Game name"), FApp::GetName());

	SubsystemNativeImpl->SetContext(TEXT("Unreal Engine"), DefaultContext);
}

void USentrySubsystem::AddGpuContext()
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	FGPUDriverInfo GpuDriverInfo = FPlatformMisc::GetGPUDriverInfo(FPlatformMisc::GetPrimaryGPUBrand());

	TMap<FString, FString> GpuContext;
	GpuContext.Add(TEXT("name"), GpuDriverInfo.DeviceDescription);
	GpuContext.Add(TEXT("vendor_name"), GpuDriverInfo.ProviderName);
	GpuContext.Add(TEXT("driver_version"), GpuDriverInfo.UserDriverVersion);

	SubsystemNativeImpl->SetContext(TEXT("gpu"), GpuContext);
}

void USentrySubsystem::AddDeviceContext()
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
		return;

	const FPlatformMemoryConstants& MemoryConstants = FPlatformMemory::GetConstants();

	TMap<FString, FString> DeviceContext;
	DeviceContext.Add(TEXT("cpu_description"), FPlatformMisc::GetCPUBrand());
	DeviceContext.Add(TEXT("number_of_cores"), FString::FromInt(FPlatformMisc::NumberOfCores()));
	DeviceContext.Add(TEXT("number_of_cores_including_hyperthreads"), FString::FromInt(FPlatformMisc::NumberOfCoresIncludingHyperthreads()));
	DeviceContext.Add(TEXT("physical_memory_size_gb"), FString::FromInt(MemoryConstants.TotalPhysicalGB));

	SubsystemNativeImpl->SetContext(TEXT("device"), DeviceContext);
}

void USentrySubsystem::PromoteTags()
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

void USentrySubsystem::ConfigureBreadcrumbs()
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

void USentrySubsystem::DisableAutomaticBreadcrumbs()
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

bool USentrySubsystem::IsCurrentBuildConfigurationEnabled()
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

bool USentrySubsystem::IsCurrentBuildTargetEnabled()
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

bool USentrySubsystem::IsCurrentPlatformEnabled()
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

bool USentrySubsystem::IsPromotedBuildsOnlyEnabled()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();

	return Settings->EnableForPromotedBuildsOnly;
}

void USentrySubsystem::ConfigureOutputDevice()
{
	OutputDevice = MakeShareable(new FSentryOutputDevice());
	if (OutputDevice)
	{
		GLog->AddOutputDevice(OutputDevice.Get());
		GLog->SerializeBacklog(OutputDevice.Get());
	}
}

void USentrySubsystem::ConfigureOutputDeviceError()
{
	OutputDeviceError = MakeShareable(new FSentryOutputDeviceError(GError));
	if (OutputDeviceError)
	{
		OnAssertDelegate = OutputDeviceError->OnAssert.AddLambda([this](const FString& Message)

		{
			SubsystemNativeImpl->CaptureAssertion(TEXT("Assertion failed"), Message);

			// Shut things down before exiting to ensure all the outgoing events are sent to Sentry
			Close();

			FPlatformMisc::RequestExit( true);
		});

		GError = OutputDeviceError.Get();
	}
}
