// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"

#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryBeforeSendHandler.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryErrorOutputDevice.h"
#include "SentryEvent.h"
#include "SentryModule.h"
#include "SentryOutputDevice.h"
#include "SentrySettings.h"
#include "SentryTraceSampler.h"
#include "SentryTransaction.h"
#include "SentryTransactionContext.h"
#include "SentryUser.h"
#include "SentryUserFeedback.h"

#include "CoreGlobals.h"
#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformDriver.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/App.h"
#include "Misc/AssertionMacros.h"
#include "Misc/CoreDelegates.h"
#include "Misc/EngineVersion.h"
#include "SentryAttachment.h"

#include "Interface/SentrySubsystemInterface.h"

#include "HAL/PlatformSentrySubsystem.h"
#include "HAL/PlatformSentryUserFeedback.h"

void USentrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SubsystemNativeImpl = MakeShareable(new FPlatformSentrySubsystem());

	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

	UE_LOG(LogSentrySdk, Log, TEXT("Sentry plugin auto initialization: %s"), *LexToString(Settings->InitAutomatically));

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
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Sentry subsystem is invalid and can't be initialized."));
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
		UE_LOG(LogSentrySdk, Log, TEXT("Sentry initialization skipped since event capturing is disabled for the current configuration/target/build in plugin settings."));
		return;
	}

	if (IsPromotedBuildsOnlyEnabled() && !FApp::GetEngineIsPromotedBuild())
	{
		UE_LOG(LogSentrySdk, Log, TEXT("Sentry initialization skipped since event capturing is disabled for the non-promoted builds in plugin settings."));
		return;
	}

	BeforeSendHandler =
		Settings->BeforeSendHandler != nullptr
			? NewObject<USentryBeforeSendHandler>(this, static_cast<UClass*>(Settings->BeforeSendHandler))
			: nullptr;

	BeforeBreadcrumbHandler =
		Settings->BeforeBreadcrumbHandler != nullptr
			? NewObject<USentryBeforeBreadcrumbHandler>(this, static_cast<UClass*>(Settings->BeforeBreadcrumbHandler))
			: nullptr;

	TraceSampler =
		Settings->TracesSampler != nullptr
			? NewObject<USentryTraceSampler>(this, static_cast<UClass*>(Settings->TracesSampler))
			: nullptr;

	SubsystemNativeImpl->InitWithSettings(Settings, BeforeSendHandler, BeforeBreadcrumbHandler, TraceSampler);

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

	OnEnsureDelegate = FCoreDelegates::OnHandleSystemEnsure.AddWeakLambda(this, [this]()
	{
		verify(SubsystemNativeImpl);

		FString EnsureMessage = GErrorHist;
		SubsystemNativeImpl->CaptureEnsure(TEXT("Ensure failed"), EnsureMessage.TrimStartAndEnd());
	});
}

void USentrySubsystem::InitializeWithSettings(const FConfigureSettingsDelegate& OnConfigureSettings)
{
	return InitializeWithSettings(FConfigureSettingsNativeDelegate::CreateUFunction(const_cast<UObject*>(OnConfigureSettings.GetUObject()), OnConfigureSettings.GetFunctionName()));
}

void USentrySubsystem::InitializeWithSettings(const FConfigureSettingsNativeDelegate& OnConfigureSettings)
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
	check(SubsystemNativeImpl);
	check(Breadcrumb);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->AddBreadcrumb(Breadcrumb->GetNativeObject());
}

void USentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->AddBreadcrumbWithParams(Message, Category, Type, Data, Level);
}

void USentrySubsystem::ClearBreadcrumbs()
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->ClearBreadcrumbs();
}

void USentrySubsystem::AddAttachment(USentryAttachment* Attachment)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->AddAttachment(Attachment->GetNativeObject());
}

void USentrySubsystem::ClearAttachments()
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->ClearAttachments();
}

FString USentrySubsystem::CaptureMessage(const FString& Message, ESentryLevel Level)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return FString();
	}

	TSharedPtr<ISentryId> SentryId = SubsystemNativeImpl->CaptureMessage(Message, Level);

	return SentryId->ToString();
}

FString USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level)
{
	return CaptureMessageWithScope(Message, FConfigureScopeNativeDelegate::CreateUFunction(const_cast<UObject*>(OnConfigureScope.GetUObject()), OnConfigureScope.GetFunctionName()), Level);
}

FString USentrySubsystem::CaptureMessageWithScope(const FString& Message, const FConfigureScopeNativeDelegate& OnConfigureScope, ESentryLevel Level)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return FString();
	}

	TSharedPtr<ISentryId> SentryId = SubsystemNativeImpl->CaptureMessageWithScope(Message, Level, FSentryScopeDelegate::CreateLambda([OnConfigureScope](TSharedPtr<ISentryScope> NativeScope)
	{
		USentryScope* UnrealScope = USentryScope::Create(NativeScope);
		OnConfigureScope.ExecuteIfBound(UnrealScope);
	}));

	return SentryId->ToString();
}

FString USentrySubsystem::CaptureEvent(USentryEvent* Event)
{
	check(SubsystemNativeImpl);
	check(Event);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return FString();
	}

	TSharedPtr<ISentryId> SentryId = SubsystemNativeImpl->CaptureEvent(Event->GetNativeObject());

	return SentryId->ToString();
}

FString USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope)
{
	return CaptureEventWithScope(Event, FConfigureScopeNativeDelegate::CreateUFunction(const_cast<UObject*>(OnConfigureScope.GetUObject()), OnConfigureScope.GetFunctionName()));
}

FString USentrySubsystem::CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeNativeDelegate& OnConfigureScope)
{
	check(SubsystemNativeImpl);
	check(Event);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return FString();
	}

	TSharedPtr<ISentryId> SentryId = SubsystemNativeImpl->CaptureEventWithScope(Event->GetNativeObject(), FSentryScopeDelegate::CreateLambda([OnConfigureScope](TSharedPtr<ISentryScope> NativeScope)
	{
		USentryScope* UnrealScope = USentryScope::Create(NativeScope);
		OnConfigureScope.ExecuteIfBound(UnrealScope);
	}));

	return SentryId->ToString();
}

void USentrySubsystem::CaptureUserFeedback(USentryUserFeedback* UserFeedback)
{
	check(SubsystemNativeImpl);
	check(UserFeedback);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->CaptureUserFeedback(UserFeedback->GetNativeObject());
}

void USentrySubsystem::CaptureUserFeedbackWithParams(const FString& EventId, const FString& Email, const FString& Comments, const FString& Name)
{
	check(SubsystemNativeImpl);
	check(!EventId.IsEmpty());

	USentryUserFeedback* UserFeedback = USentryUserFeedback::Create(CreateSharedSentryUserFeedback(EventId));
	check(UserFeedback);

	UserFeedback->SetEmail(Email);
	UserFeedback->SetComment(Comments);
	UserFeedback->SetName(Name);

	CaptureUserFeedback(UserFeedback);
}

void USentrySubsystem::SetUser(USentryUser* User)
{
	check(SubsystemNativeImpl);
	check(User);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->SetUser(User->GetNativeObject());
}

void USentrySubsystem::RemoveUser()
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->RemoveUser();
}

void USentrySubsystem::SetContext(const FString& Key, const TMap<FString, FSentryVariant>& Values)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->SetContext(Key, Values);
}

void USentrySubsystem::SetTag(const FString& Key, const FString& Value)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->SetTag(Key, Value);
}

void USentrySubsystem::RemoveTag(const FString& Key)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->RemoveTag(Key);
}

void USentrySubsystem::SetLevel(ESentryLevel Level)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->SetLevel(Level);
}

void USentrySubsystem::StartSession()
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->StartSession();
}

void USentrySubsystem::EndSession()
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	SubsystemNativeImpl->EndSession();
}

USentryTransaction* USentrySubsystem::StartTransaction(const FString& Name, const FString& Operation)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return nullptr;
	}

	TSharedPtr<ISentryTransaction> SentryTransaction = SubsystemNativeImpl->StartTransaction(Name, Operation);
	check(SentryTransaction);

	return USentryTransaction::Create(SentryTransaction);
}

USentryTransaction* USentrySubsystem::StartTransactionWithContext(USentryTransactionContext* Context)
{
	check(SubsystemNativeImpl);
	check(Context);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return nullptr;
	}

	TSharedPtr<ISentryTransaction> SentryTransaction = SubsystemNativeImpl->StartTransactionWithContext(Context->GetNativeObject());
	check(SentryTransaction);

	return USentryTransaction::Create(SentryTransaction);
}

USentryTransaction* USentrySubsystem::StartTransactionWithContextAndTimestamp(USentryTransactionContext* Context, int64 Timestamp)
{
	check(SubsystemNativeImpl);
	check(Context);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return nullptr;
	}

	TSharedPtr<ISentryTransaction> SentryTransaction = SubsystemNativeImpl->StartTransactionWithContextAndTimestamp(Context->GetNativeObject(), Timestamp);
	check(SentryTransaction);

	return USentryTransaction::Create(SentryTransaction);
}

USentryTransaction* USentrySubsystem::StartTransactionWithContextAndOptions(USentryTransactionContext* Context, const TMap<FString, FString>& Options)
{
	check(SubsystemNativeImpl);
	check(Context);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return nullptr;
	}

	TSharedPtr<ISentryTransaction> SentryTransaction = SubsystemNativeImpl->StartTransactionWithContextAndOptions(Context->GetNativeObject(), Options);
	check(SentryTransaction);

	return USentryTransaction::Create(SentryTransaction);
}

USentryTransactionContext* USentrySubsystem::ContinueTrace(const FString& SentryTrace, const TArray<FString>& BaggageHeaders)
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
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
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	TMap<FString, FSentryVariant> DefaultContext;
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
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	FGPUDriverInfo GpuDriverInfo = FPlatformMisc::GetGPUDriverInfo(FPlatformMisc::GetPrimaryGPUBrand());

	TMap<FString, FSentryVariant> GpuContext;
	GpuContext.Add(TEXT("name"), GpuDriverInfo.DeviceDescription);
	GpuContext.Add(TEXT("vendor_name"), GpuDriverInfo.ProviderName);
	GpuContext.Add(TEXT("driver_version"), GpuDriverInfo.UserDriverVersion);

	SubsystemNativeImpl->SetContext(TEXT("gpu"), GpuContext);
}

void USentrySubsystem::AddDeviceContext()
{
	check(SubsystemNativeImpl);

	if (!SubsystemNativeImpl || !SubsystemNativeImpl->IsEnabled())
	{
		return;
	}

	const FPlatformMemoryConstants& MemoryConstants = FPlatformMemory::GetConstants();

	TMap<FString, FSentryVariant> DeviceContext;
	DeviceContext.Add(TEXT("cpu_description"), FPlatformMisc::GetCPUBrand());
	DeviceContext.Add(TEXT("number_of_cores"), FString::FromInt(FPlatformMisc::NumberOfCores()));
	DeviceContext.Add(TEXT("number_of_cores_including_hyperthreads"), FString::FromInt(FPlatformMisc::NumberOfCoresIncludingHyperthreads()));
	DeviceContext.Add(TEXT("physical_memory_size_gb"), FString::FromInt(MemoryConstants.TotalPhysicalGB));

	SubsystemNativeImpl->SetContext(TEXT("device"), DeviceContext);
}

void USentrySubsystem::PromoteTags()
{
	check(SubsystemNativeImpl);

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
			AddBreadcrumbWithParams(TEXT("PreLoadMap"), TEXT("Unreal"), TEXT("Default"), { { TEXT("Map"), MapName } }, ESentryLevel::Info);
		});
	}

	if (Settings->AutomaticBreadcrumbs.bOnMapLoaded)
	{
		PostLoadMapDelegate = FCoreUObjectDelegates::PostLoadMapWithWorld.AddWeakLambda(this, [this](UWorld* World)
		{
			if (World)
			{
				AddBreadcrumbWithParams(TEXT("PostLoadMapWithWorld"), TEXT("Unreal"), TEXT("Default"), { { TEXT("Map"), World->GetMapName() } }, ESentryLevel::Info);
			}
			else
			{
				AddBreadcrumbWithParams(TEXT("PostLoadMapWithWorld"), TEXT("Unreal"), TEXT("Default"), { { TEXT("Error"), TEXT("Map load failed") } }, ESentryLevel::Error);
			}
		});
	}

	if (Settings->AutomaticBreadcrumbs.bOnGameStateClassChanged)
	{
		GameStateChangedDelegate = FCoreDelegates::GameStateClassChanged.AddWeakLambda(this, [this](const FString& GameState)
		{
			AddBreadcrumbWithParams(TEXT("GameStateClassChanged"), TEXT("Unreal"), TEXT("Default"), { { TEXT("GameState"), GameState } }, ESentryLevel::Info);
		});
	}

	if (Settings->AutomaticBreadcrumbs.bOnUserActivityStringChanged)
	{
		UserActivityChangedDelegate = FCoreDelegates::UserActivityStringChanged.AddWeakLambda(this, [this](const FString& Activity)
		{
			AddBreadcrumbWithParams(TEXT("UserActivityStringChanged"), TEXT("Unreal"), TEXT("Default"), { { TEXT("Activity"), Activity } }, ESentryLevel::Info);
		});
	}

	if (Settings->AutomaticBreadcrumbs.bOnGameSessionIDChanged)
	{
		GameSessionIDChangedDelegate = FCoreDelegates::GameSessionIDChanged.AddWeakLambda(this, [this](const FString& SessionId)
		{
			AddBreadcrumbWithParams(TEXT("GameSessionIDChanged"), TEXT("Unreal"), TEXT("Default"), { { TEXT("Session ID"), SessionId } }, ESentryLevel::Info);
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

	switch (FApp::GetBuildConfiguration())
	{
	case EBuildConfiguration::Debug:
		return Settings->EnableBuildConfigurations.bEnableDebug;
	case EBuildConfiguration::DebugGame:
		return Settings->EnableBuildConfigurations.bEnableDebugGame;
	case EBuildConfiguration::Development:
		return Settings->EnableBuildConfigurations.bEnableDevelopment;
	case EBuildConfiguration::Shipping:
		return Settings->EnableBuildConfigurations.bEnableShipping;
	case EBuildConfiguration::Test:
		return Settings->EnableBuildConfigurations.bEnableTest;
	default:
		return false;
	}
}

bool USentrySubsystem::IsCurrentBuildTargetEnabled() const
{
	const USentrySettings* Settings = FSentryModule::Get().GetSettings();
	check(Settings);

	switch (FApp::GetBuildTargetType())
	{
	case EBuildTargetType::Game:
		return Settings->EnableBuildTargets.bEnableGame;
	case EBuildTargetType::Server:
		return Settings->EnableBuildTargets.bEnableServer;
	case EBuildTargetType::Client:
		return Settings->EnableBuildTargets.bEnableClient;
	case EBuildTargetType::Editor:
		return Settings->EnableBuildTargets.bEnableEditor;
	case EBuildTargetType::Program:
		return Settings->EnableBuildTargets.bEnableProgram;
	default:
		return false;
	}
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
		OnAssertDelegate = OutputDeviceError->OnAssert.AddWeakLambda(this, [this](const FString& Message)
		{
			check(SubsystemNativeImpl);
			SubsystemNativeImpl->HandleAssert();
		});
		GError = OutputDeviceError.Get();
	}
}
