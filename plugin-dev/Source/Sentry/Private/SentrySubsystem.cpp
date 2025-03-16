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

#include "HAL/PlatformSentrySubsystem.h"
#include "HAL/PlatformSentryUserFeedback.h"

void USentrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SubsystemNativeImpl = MakeShareable(new FPlatformSentrySubsystem());

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

	UE_LOG(LogSentrySdk, Log, TEXT("Sentry plugin auto initialization: %s"), LexToString(Settings->InitAutomatically));

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
	if (!verify(SubsystemNativeImpl))
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
	check(Settings);

	if (Settings->Dsn.IsEmpty())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry requires minimal configuration for its initialization - please provide the DSN in plugin settings."));
		return;
	}

	if (!IsCurrentBuildConfigurationEnabled() || !IsCurrentBuildTargetEnabled())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry initialization skipped since event capturing is disabled for the current configuration/target/build in plugin settings."));
		return;
	}

	if (IsPromotedBuildsOnlyEnabled() && !FApp::GetEngineIsPromotedBuild())
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Sentry initialization skipped since event capturing is disabled for the non-promoted builds in plugin settings."));
		return;
	}

	const UClass* BeforeSendHandlerClass = Settings->BeforeSendHandler != nullptr
		? static_cast<UClass*>(Settings->BeforeSendHandler)
		: USentryBeforeSendHandler::StaticClass();

	BeforeSendHandler = NewObject<USentryBeforeSendHandler>(this, BeforeSendHandlerClass);
	check(BeforeSendHandler);

	const UClass* TraceSamplerClass = Settings->TracesSampler != nullptr
		? static_cast<UClass*>(Settings->TracesSampler)
		: USentryTraceSampler::StaticClass();

	TraceSampler = NewObject<USentryTraceSampler>(this, TraceSamplerClass);
	check(TraceSampler);

	SubsystemNativeImpl->InitWithSettings(Settings, BeforeSendHandler, TraceSampler);

	if (!SubsystemNativeImpl->IsEnabled())
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
	ConfigureErrorOutputDevice();

	OnEnsureDelegate = FCoreDelegates::OnHandleSystemEnsure.AddWeakLambda(this, [SubsystemNativeImpl]()
	{
		verify(SubsystemNativeImpl);

		FString EnsureMessage = GErrorHist;
		SubsystemNativeImpl->CaptureEnsure(TEXT("Ensure failed"), EnsureMessage.TrimStartAndEnd());
	});
}

void USentrySubsystem::InitializeWithSettings(const FConfigureSettingsDelegate& OnConfigureSettings)
{
	USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

	OnConfigureSettings.ExecuteIfBound(Settings);

	Initialize();
}

void USentrySubsystem::Close()
{
	if (GLog && OutputDevice)
	{
		GLog->RemoveOutputDevice(OutputDevice.Get());
		OutputDevice = nullptr;
	}

	if (GError && OutputDeviceError)
	{
		if (OnAssertDelegate.IsValid())
		{
			OutputDeviceError->OnAssert.Remove(OnAssertDelegate);
			OnAssertDelegate.Reset();
		}

		GError = OutputDeviceError->GetParentDevice();
		OutputDeviceError = nullptr;
	}

	if (OnEnsureDelegate.IsValid())
	{
		FCoreDelegates::OnHandleSystemEnsure.Remove(OnEnsureDelegate);
		OnEnsureDelegate.Reset();
	}

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->Close();
}

bool USentrySubsystem::IsEnabled() const
{
	return SubsystemNativeImpl ? SubsystemNativeImpl->IsEnabled() : false;
}

ESentryCrashedLastRun USentrySubsystem::IsCrashedLastRun() const
{
	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return ESentryCrashedLastRun::NotEvaluated;
	}

	return SubsystemNativeImpl->IsCrashedLastRun();
}

void USentrySubsystem::AddBreadcrumb(USentryBreadcrumb* Breadcrumb)
{
	check(Breadcrumb);

	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->AddBreadcrumb(Breadcrumb->GetNativeObject());
}

void USentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->AddBreadcrumbWithParams(Message, Category, Type, Data, Level);
}

void USentrySubsystem::ClearBreadcrumbs()
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->ClearBreadcrumbs();
}

USentryId* USentrySubsystem::CaptureMessage(const FString& Message, ESentryLevel Level)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TSharedPtr<ISentryId> SentryId = SubsystemNativeImpl->CaptureMessage(Message, Level);

	return USentryId::Create(SentryId);
}

USentryId* USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level)
{
	return CaptureMessageWithScope(Message, FConfigureScopeNativeDelegate::CreateUFunction(const_cast<UObject*>(OnConfigureScope.GetUObject()), OnConfigureScope.GetFunctionName()), Level);
}

USentryId* USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeNativeDelegate& OnConfigureScope, ESentryLevel Level)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TSharedPtr<ISentryId> SentryId = SubsystemNativeImpl->CaptureMessageWithScope(Message, FSentryScopeDelegate::CreateLambda([OnConfigureScope](TSharedPtr<ISentryScope> NativeScope)
	{
		USentryScope* UnrealScope = USentryScope::Create(NativeScope);
		OnConfigureScope.ExecuteIfBound(UnrealScope);
	}), Level);

	return USentryId::Create(SentryId);
}

USentryId* USentrySubsystem::CaptureEvent(USentryEvent* Event)
{
	check(Event);

	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TSharedPtr<ISentryId> SentryId = SubsystemNativeImpl->CaptureEvent(Event->GetNativeObject());

	return USentryId::Create(SentryId);
}

USentryId* USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope)
{
	return CaptureEventWithScope(Event, FConfigureScopeNativeDelegate::CreateUFunction(const_cast<UObject*>(OnConfigureScope.GetUObject()), OnConfigureScope.GetFunctionName()));
}

USentryId* USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeNativeDelegate& OnConfigureScope)
{
	check(Event);

	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TSharedPtr<ISentryId> SentryId = SubsystemNativeImpl->CaptureEventWithScope(Event->GetNativeObject(), FSentryScopeDelegate::CreateLambda([OnConfigureScope](TSharedPtr<ISentryScope> NativeScope)
	{
		USentryScope* UnrealScope = USentryScope::Create(NativeScope);
		OnConfigureScope.ExecuteIfBound(UnrealScope);
	}));

	return USentryId::Create(SentryId);
}

void USentrySubsystem::CaptureUserFeedback(USentryUserFeedback* UserFeedback)
{
	check(UserFeedback);

	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->CaptureUserFeedback(UserFeedback->GetNativeObject());
}

void USentrySubsystem::CaptureUserFeedbackWithParams(USentryId* EventId, const FString& Email, const FString& Comments, const FString& Name)
{
	check(EventId);

	USentryUserFeedback* UserFeedback = USentryUserFeedback::Create(CreateSharedSentryUserFeedback(EventId->GetNativeObject()));
	check(UserFeedback);

	UserFeedback->SetEmail(Email);
	UserFeedback->SetComment(Comments);
	UserFeedback->SetName(Name);

	CaptureUserFeedback(UserFeedback);
}

void USentrySubsystem::SetUser(USentryUser* User)
{
	check(User);

	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->SetUser(User->GetNativeObject());
}

void USentrySubsystem::RemoveUser()
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->RemoveUser();
}

void USentrySubsystem::ConfigureScope(const FConfigureScopeDelegate& OnConfigureScope)
{
	ConfigureScope(FConfigureScopeNativeDelegate::CreateUFunction(const_cast<UObject*>(OnConfigureScope.GetUObject()), OnConfigureScope.GetFunctionName()));
}

void USentrySubsystem::ConfigureScope(const FConfigureScopeNativeDelegate& OnConfigureScope)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->ConfigureScope(FSentryScopeDelegate::CreateLambda([OnConfigureScope](TSharedPtr<ISentryScope> NativeScope)
	{
		USentryScope* UnrealScope = USentryScope::Create(NativeScope);
		OnConfigureScope.ExecuteIfBound(UnrealScope);
	}));
}

void USentrySubsystem::SetContext(const FString& Key, const TMap<FString, FString>& Values)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

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
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->RemoveTag(Key);
}

void USentrySubsystem::SetLevel(ESentryLevel Level)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->SetLevel(Level);
}

void USentrySubsystem::StartSession()
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->StartSession();
}

void USentrySubsystem::EndSession()
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return;
	}

	SubsystemNativeImpl->EndSession();
}

USentryTransaction* USentrySubsystem::StartTransaction(const FString& Name, const FString& Operation)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TSharedPtr<ISentryTransaction> SentryTransaction = SubsystemNativeImpl->StartTransaction(Name, Operation);
	check(SentryTransaction);

	return USentryTransaction::Create(SentryTransaction);
}

USentryTransaction* USentrySubsystem::StartTransactionWithContext(USentryTransactionContext* Context)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TSharedPtr<ISentryTransaction> SentryTransaction = SubsystemNativeImpl->StartTransactionWithContext(Context->GetNativeObject());
	check(SentryTransaction);

	return USentryTransaction::Create(SentryTransaction);
}

USentryTransaction* USentrySubsystem::StartTransactionWithContextAndTimestamp(USentryTransactionContext* Context, int64 Timestamp)
{
	check(Context);

	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TSharedPtr<ISentryTransaction> SentryTransaction = SubsystemNativeImpl->StartTransactionWithContextAndTimestamp(Context->GetNativeObject(), Timestamp);
	check(SentryTransaction);

	return USentryTransaction::Create(SentryTransaction);
}

USentryTransaction* USentrySubsystem::StartTransactionWithContextAndOptions(USentryTransactionContext* Context, const TMap<FString, FString>& Options)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TSharedPtr<ISentryTransaction> SentryTransaction = SubsystemNativeImpl->StartTransactionWithContextAndOptions(Context->GetNativeObject(), Options);
	check(SentryTransaction);

	return USentryTransaction::Create(SentryTransaction);
}

USentryTransactionContext* USentrySubsystem::ContinueTrace(const FString& SentryTrace, const TArray<FString>& BaggageHeaders)
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TSharedPtr<ISentryTransactionContext> SentryTransactionContext = SubsystemNativeImpl->ContinueTrace(SentryTrace, BaggageHeaders);
	check(SentryTransactionContext);

	return USentryTransactionContext::Create(SentryTransactionContext);
}

bool USentrySubsystem::IsSupportedForCurrentSettings() const
{
	if (!IsCurrentBuildConfigurationEnabled() || !IsCurrentBuildTargetEnabled())
	{
		return false;
	}

	if (IsPromotedBuildsOnlyEnabled() && !FApp::GetEngineIsPromotedBuild())
	{
		return false;
	}

	return true;
}

void USentrySubsystem::AddDefaultContext()
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	TMap<FString, FString> DefaultContext;
	DefaultContext.Add(TEXT("Engine version"), FEngineVersion::Current().ToString(EVersionComponent::Changelist));
	DefaultContext.Add(TEXT("Plugin version"), FSentryModule::Get().GetPluginVersion());
	DefaultContext.Add(TEXT("Is Marketplace version"), LexToString(FSentryModule::Get().IsMarketplaceVersion()));
	DefaultContext.Add(TEXT("Configuration"), LexToString(FApp::GetBuildConfiguration()));
	DefaultContext.Add(TEXT("Target Type"), LexToString(FApp::GetBuildTargetType()));
	DefaultContext.Add(TEXT("Engine mode"), FGenericPlatformMisc::GetEngineMode());
	DefaultContext.Add(TEXT("Is game"), LexToString(FApp::IsGame()));
	DefaultContext.Add(TEXT("Is standalone"), LexToString(FApp::IsStandalone()));
	DefaultContext.Add(TEXT("Is unattended"), LexToString(FApp::IsUnattended()));
	DefaultContext.Add(TEXT("Game name"), FApp::GetName());

	SubsystemNativeImpl->SetContext(TEXT("Unreal Engine"), DefaultContext);
}

void USentrySubsystem::AddGpuContext()
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

	FGPUDriverInfo GpuDriverInfo = FPlatformMisc::GetGPUDriverInfo(FPlatformMisc::GetPrimaryGPUBrand());

	TMap<FString, FString> GpuContext;
	GpuContext.Add(TEXT("name"), GpuDriverInfo.DeviceDescription);
	GpuContext.Add(TEXT("vendor_name"), GpuDriverInfo.ProviderName);
	GpuContext.Add(TEXT("driver_version"), GpuDriverInfo.UserDriverVersion);

	SubsystemNativeImpl->SetContext(TEXT("gpu"), GpuContext);
}

void USentrySubsystem::AddDeviceContext()
{
	if (!verify(SubsytemNativeImpl) || !verify(SubsystemNativeImpl->IsEnabled()))
	{
		return nullptr;
	}

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
	check(Settings);

	if (Settings->TagsPromotion.bPromoteBuildConfiguration)
	{
		SubsystemNativeImpl->SetTag(TEXT("Configuration"), LexToString(FApp::GetBuildConfiguration()));
	}

	if (Settings->TagsPromotion.bPromoteTargetType)
	{
		SubsystemNativeImpl->SetTag(TEXT("Target Type"), LexToString(FApp::GetBuildTargetType()));
	}

	if (Settings->TagsPromotion.bPromoteEngineMode)
	{
		SubsystemNativeImpl->SetTag(TEXT("Engine Mode"), FGenericPlatformMisc::GetEngineMode());
	}

	if (Settings->TagsPromotion.bPromoteIsGame)
	{
		SubsystemNativeImpl->SetTag(TEXT("Is game"), LexToString(FApp::IsGame()));
	}

	if (Settings->TagsPromotion.bPromoteIsStandalone)
	{
		SubsystemNativeImpl->SetTag(TEXT("Is standalone"), LexToString(FApp::IsStandalone()));
	}

	if (Settings->TagsPromotion.bPromoteIsUnattended)
	{
		SubsystemNativeImpl->SetTag(TEXT("Is unattended"), LexToString(FApp::IsUnattended()));
	}
}

void USentrySubsystem::ConfigureBreadcrumbs()
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

	if (Settings->AutomaticBreadcrumbs.bOnMapLoadingStarted)
	{
		PreLoadMapDelegate = FCoreUObjectDelegates::PreLoadMap.AddWeakLambda(this, [this](const FString& MapName)
		{
			AddBreadcrumbWithParams(TEXT("PreLoadMap"), TEXT("Unreal"), TEXT("Default"), {{TEXT("Map"), MapName}}, ESentryLevel::Info);
		});
	}

	if (Settings->AutomaticBreadcrumbs.bOnMapLoaded)
	{
		PostLoadMapDelegate = FCoreUObjectDelegates::PostLoadMapWithWorld.AddWeakLambda(this, [this](UWorld* World)
		{
			if (World)
			{
				AddBreadcrumbWithParams(TEXT("PostLoadMapWithWorld"), TEXT("Unreal"), TEXT("Default"), {{TEXT("Map"), World->GetMapName()}}, ESentryLevel::Info);
			}
			else
			{
				AddBreadcrumbWithParams(TEXT("PostLoadMapWithWorld"), TEXT("Unreal"), TEXT("Default"), {{TEXT("Error"), TEXT("Map load failed")}}, ESentryLevel::Error);
			}
		});
	}

	if (Settings->AutomaticBreadcrumbs.bOnGameStateClassChanged)
	{
		GameStateChangedDelegate = FCoreDelegates::GameStateClassChanged.AddWeakLambda(this, [this](const FString& GameState)
		{
			AddBreadcrumbWithParams(TEXT("GameStateClassChanged"), TEXT("Unreal"), TEXT("Default"), {{TEXT("GameState"), GameState}}, ESentryLevel::Info);
		});
	}

	if (Settings->AutomaticBreadcrumbs.bOnUserActivityStringChanged)
	{
		UserActivityChangedDelegate = FCoreDelegates::UserActivityStringChanged.AddWeakLambda(this, [this](const FString& Activity)
		{
			AddBreadcrumbWithParams(TEXT("UserActivityStringChanged"), TEXT("Unreal"), TEXT("Default"), {{TEXT("Activity"), Activity}}, ESentryLevel::Info);
		});
	}

	if (Settings->AutomaticBreadcrumbs.bOnGameSessionIDChanged)
	{
		GameSessionIDChangedDelegate = FCoreDelegates::GameSessionIDChanged.AddWeakLambda(this, [this](const FString& SessionId)
		{
			AddBreadcrumbWithParams(TEXT("GameSessionIDChanged"), TEXT("Unreal"), TEXT("Default"), {{TEXT("Session ID"), SessionId}}, ESentryLevel::Info);
		});
	}
}

void USentrySubsystem::DisableAutomaticBreadcrumbs()
{
	if (PreLoadMapDelegate.IsValid())
	{
		FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapDelegate);
	}

	if (PostLoadMapDelegate.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapDelegate);
	}

	if (GameStateChangedDelegate.IsValid())
	{
		FCoreDelegates::GameStateClassChanged.Remove(GameStateChangedDelegate);
	}

	if (UserActivityChangedDelegate.IsValid())
	{
		FCoreDelegates::UserActivityStringChanged.Remove(UserActivityChangedDelegate);
	}

	if (GameSessionIDChangedDelegate.IsValid())
	{
		FCoreDelegates::GameSessionIDChanged.Remove(GameSessionIDChangedDelegate);
	}
}

bool USentrySubsystem::IsCurrentBuildConfigurationEnabled() const
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

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
		break;
	}

	return IsBuildConfigurationEnabled;
}

bool USentrySubsystem::IsCurrentBuildTargetEnabled() const
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

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
		break;
	}

	return IsBuildTargetTypeEnabled;
}

bool USentrySubsystem::IsPromotedBuildsOnlyEnabled() const
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

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

void USentrySubsystem::ConfigureErrorOutputDevice()
{
	OutputDeviceError = MakeShareable(new FSentryErrorOutputDevice(GError));
	if (OutputDeviceError)
	{
#if PLATFORM_ANDROID
		OnAssertDelegate = OutputDeviceError->OnAssert.AddWeakLambda(this, [this](const FString& Message)
		{
			GError->HandleError();
			PLATFORM_BREAK();
		});
#endif // PLATFORM_ANDROID

		GError = OutputDeviceError.Get();
	}
}
