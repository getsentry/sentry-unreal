// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentrySubsystem.h"

#if !USE_SENTRY_NATIVE

#include "AppleSentryAttachment.h"
#include "AppleSentryBreadcrumb.h"
#include "AppleSentryEvent.h"
#include "AppleSentryFeedback.h"
#include "AppleSentryId.h"
#include "AppleSentryLog.h"
#include "AppleSentryMetric.h"
#include "AppleSentryReplayEnvelope.h"
#include "AppleSentrySamplingContext.h"
#include "AppleSentryScope.h"
#include "AppleSentryTransaction.h"
#include "AppleSentryTransactionContext.h"
#include "AppleSentryUser.h"

#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryBeforeLogHandler.h"
#include "SentryBeforeMetricHandler.h"
#include "SentryBeforeSendHandler.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryEvent.h"
#include "SentryLog.h"
#include "SentryMetric.h"
#include "SentrySamplingContext.h"
#include "SentrySettings.h"
#include "SentryTraceSampler.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

#include "Utils/SentryCallbackUtils.h"
#include "Utils/SentryFileUtils.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/FileManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"

void FAppleSentrySubsystem::InitWithSettings(const USentrySettings* settings, const FSentryCallbackHandlers& callbackHandlers)
{
	USentryBeforeSendHandler* beforeSendHandler = callbackHandlers.BeforeSendHandler;
	USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler = callbackHandlers.BeforeBreadcrumbHandler;
	USentryBeforeLogHandler* beforeLogHandler = callbackHandlers.BeforeLogHandler;
	USentryBeforeMetricHandler* beforeMetricHandler = callbackHandlers.BeforeMetricHandler;
	USentryTraceSampler* traceSampler = callbackHandlers.TraceSampler;

	isScreenshotAttachmentEnabled = settings->AttachScreenshot;
	isGameLogAttachmentEnabled = settings->EnableAutoLogAttachment;
	isSessionReplayAttachmentEnabled = settings->AttachSessionReplay;
	maxAttachmentSize = settings->MaxAttachmentSize;

	FString prevSessionReplayPath;
	FString prevSessionReplaySidecarPath;

	if (settings->AttachSessionReplay)
	{
		// Capture the previous run's replay paths before starting the recorder to
		// avoid a race where onLastRunStatusDetermined picks up the current session's files
		if (!GetLatestSessionReplay(prevSessionReplayPath, prevSessionReplaySidecarPath))
		{
			UE_LOG(LogSentrySdk, Log, TEXT("No session replays from the previous session were found."));
		}
	}

	[[[SENTRY_APPLE_CLASS(SentryObjCSDK) internal] sdk] setName:@"sentry.cocoa.unreal"];

	dispatch_group_t sentryDispatchGroup = dispatch_group_create();
	dispatch_group_enter(sentryDispatchGroup);
	dispatch_async(dispatch_get_main_queue(), ^{
		[SENTRY_APPLE_CLASS(SentryObjCSDK) startWithConfigureOptions:^(SentryObjCOptions* options) {
			options.dsn = settings->GetEffectiveDsn().GetNSString();
			options.releaseName = settings->GetEffectiveRelease().GetNSString();
			options.environment = settings->GetEffectiveEnvironment().GetNSString();
			options.dist = settings->Dist.GetNSString();
			options.enableAutoSessionTracking = settings->EnableAutoSessionTracking;
			options.sessionTrackingIntervalMillis = settings->SessionTimeout;
			options.attachStacktrace = settings->AttachStacktrace;
			options.debug = settings->Debug;
			options.sampleRate = [NSNumber numberWithFloat:settings->SampleRate];
			options.maxBreadcrumbs = settings->MaxBreadcrumbs;
			options.sendDefaultPii = settings->SendDefaultPii;
			options.maxAttachmentSize = settings->MaxAttachmentSize;
			options.enableLogs = settings->EnableStructuredLogging;
			options.enableMetrics = settings->EnableMetrics;
#if SENTRY_OBJC_UIKIT_AVAILABLE
			options.attachScreenshot = settings->AttachScreenshot;
#endif
			options.onLastRunStatusDetermined = ^(SentryObjCLastRunStatus status, SentryObjCEvent* event) {
				if (status != SentryObjCLastRunStatusDidCrash || event == nil)
				{
					// No crash to attach to — drop the previous run's replay files so they
					// don't linger and confuse the next session's "latest" detection
					if (FPaths::FileExists(prevSessionReplayPath))
					{
						IFileManager::Get().Delete(*prevSessionReplayPath);
					}
					if (FPaths::FileExists(prevSessionReplaySidecarPath))
					{
						IFileManager::Get().Delete(*prevSessionReplaySidecarPath);
					}
					return;
				}
				if (settings->AttachScreenshot)
				{
					// If a screenshot was captured during assertion/crash in the previous app run
					// find the most recent one and upload it to Sentry.
					UploadScreenshotForEvent(MakeShareable(new FAppleSentryId(event.eventId)), GetLatestScreenshot());
				}
				if (settings->EnableAutoLogAttachment)
				{
					// Unreal creates game log backups automatically on every app run. If logging is enabled for current configuration, SDK can
					// find the most recent one and upload it to Sentry.
					UploadGameLogForEvent(MakeShareable(new FAppleSentryId(event.eventId)), GetLatestGameLog());
				}
				if (settings->AttachSessionReplay)
				{
					// If a replay was captured during the previous app run upload it to Sentry.
					UploadSessionReplayForEvent(MakeShareable(new FAppleSentryId(event.eventId)), prevSessionReplayPath);

					if (FPaths::FileExists(prevSessionReplayPath) && FPaths::FileExists(prevSessionReplaySidecarPath))
					{
						// Send the structured replay envelope for the crash.
						FAppleSentryReplayEnvelope::CaptureForCrashEvent(event, prevSessionReplayPath, prevSessionReplaySidecarPath);
					}

					// Clean up the replay files processed above.
					if (FPaths::FileExists(prevSessionReplayPath))
					{
						IFileManager::Get().Delete(*prevSessionReplayPath);
					}
					if (FPaths::FileExists(prevSessionReplaySidecarPath))
					{
						IFileManager::Get().Delete(*prevSessionReplaySidecarPath);
					}
				}
			};
			for (auto it = settings->InAppInclude.CreateConstIterator(); it; ++it)
			{
				[options addInAppInclude:it->GetNSString()];
			}
			options.enableAppHangTracking = settings->EnableAppNotRespondingTracking;
			options.appHangTimeoutInterval = settings->AppNotRespondingTimeout;
			if (settings->EnableOfflineCaching)
			{
				options.maxCacheItems = settings->CacheMaxItems;
			}
			if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::UniformSampleRate)
			{
				options.tracesSampleRate = [NSNumber numberWithFloat:settings->TracesSampleRate];
			}
			if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::TracesSampler && traceSampler != nullptr)
			{
				options.tracesSampler = ^NSNumber*(SentryObjCSamplingContext* samplingContext) {
					if (!SentryCallbackUtils::IsCallbackSafeToRun())
					{
						// Falling back to default sampling value without calling a custom sampling function
						return nil;
					}

					TSentryCallbackGuard<USentryTraceSampler> ReentrancyGuard;
					if (ReentrancyGuard.IsReentrant())
					{
						return nil;
					}

					USentrySamplingContext* Context = USentrySamplingContext::Create(MakeShareable(new FAppleSentrySamplingContext(samplingContext)));

					float samplingValue;
					return traceSampler->Sample(Context, samplingValue) ? [NSNumber numberWithFloat:samplingValue] : nil;
				};
			}
			if (beforeBreadcrumbHandler != nullptr)
			{
				options.beforeBreadcrumb = ^SentryObjCBreadcrumb*(SentryObjCBreadcrumb* breadcrumb) {
					if (!SentryCallbackUtils::IsCallbackSafeToRun())
					{
						// Breadcrumb will be added without calling a `beforeBreadcrumb` handler
						return breadcrumb;
					}

					TSentryCallbackGuard<USentryBeforeBreadcrumbHandler> ReentrancyGuard;
					if (ReentrancyGuard.IsReentrant())
					{
						return breadcrumb;
					}

					USentryBreadcrumb* BreadcrumbToProcess = USentryBreadcrumb::Create(MakeShareable(new FAppleSentryBreadcrumb(breadcrumb)));

					USentryBreadcrumb* ProcessedBreadcrumb = beforeBreadcrumbHandler->HandleBeforeBreadcrumb(BreadcrumbToProcess, nullptr);

					return ProcessedBreadcrumb ? breadcrumb : nullptr;
				};
			}
			if (beforeLogHandler != nullptr)
			{
				options.beforeSendLog = ^SentryObjCLog*(SentryObjCLog* log) {
					if (!SentryCallbackUtils::IsCallbackSafeToRun())
					{
						// Log will be added without calling a `onBeforeLog` handler
						return log;
					}

					TSentryCallbackGuard<USentryBeforeLogHandler> ReentrancyGuard;
					if (ReentrancyGuard.IsReentrant())
					{
						return log;
					}

					USentryLog* LogToProcess = USentryLog::Create(MakeShareable(new FAppleSentryLog(log)));

					USentryLog* ProcessedLog = beforeLogHandler->HandleBeforeLog(LogToProcess);

					return ProcessedLog ? log : nullptr;
				};
			}
			if (beforeMetricHandler != nullptr)
			{
				options.beforeSendMetric = ^SentryObjCMetric*(SentryObjCMetric* metric) {
					if (!SentryCallbackUtils::IsCallbackSafeToRun())
					{
						// Metric will be sent without calling a `beforeSendMetric` handler
						return metric;
					}

					TSentryCallbackGuard<USentryBeforeMetricHandler> ReentrancyGuard;
					if (ReentrancyGuard.IsReentrant())
					{
						return metric;
					}

					USentryMetric* MetricToProcess = USentryMetric::Create(MakeShareable(new FAppleSentryMetric(metric)));

					USentryMetric* ProcessedMetric = beforeMetricHandler->HandleBeforeMetric(MetricToProcess);

					return ProcessedMetric ? metric : nullptr;
				};
			}
			if (beforeSendHandler != nullptr)
			{
				options.beforeSend = ^SentryObjCEvent*(SentryObjCEvent* event) {
					if (!SentryCallbackUtils::IsCallbackSafeToRun())
					{
						// Event will be sent without calling a `onBeforeSend` handler
						return event;
					}

					TSentryCallbackGuard<USentryBeforeSendHandler> ReentrancyGuard;
					if (ReentrancyGuard.IsReentrant())
					{
						return event;
					}

					USentryEvent* EventToProcess = USentryEvent::Create(MakeShareable(new FAppleSentryEvent(event)));

					USentryEvent* ProcessedEvent = beforeSendHandler->HandleBeforeSend(EventToProcess, nullptr);

					return ProcessedEvent ? event : nullptr;
				};
			}
		}];

		dispatch_group_leave(sentryDispatchGroup);
	});

	// Wait synchronously until sentry-cocoa initialization finished in main thread
	dispatch_group_wait(sentryDispatchGroup, DISPATCH_TIME_FOREVER);
	dispatch_release(sentryDispatchGroup);

	if (IsEnabled())
	{
#ifdef USE_SENTRY_SESSION_REPLAY
		if (settings->AttachSessionReplay)
		{
			SessionReplayId = FGuid::NewGuid().ToString(EGuidFormats::Digits).ToLower();

			SessionReplay = MakeUnique<FSentrySessionReplayRecorder>();
			if (SessionReplay->Initialize(settings, SessionReplayId, GetReplayPath()))
			{
				SetContext(TEXT("replay"), { { TEXT("replay_id"), FSentryVariant(SessionReplayId) } });
			}
			else
			{
				SessionReplay.Reset();
				SessionReplayId.Reset();
			}
		}
#endif
	}
}

void FAppleSentrySubsystem::Close()
{
#ifdef USE_SENTRY_SESSION_REPLAY
	if (SessionReplay)
	{
		SessionReplay->Shutdown();
		SessionReplay.Reset();
	}
#endif

	[SENTRY_APPLE_CLASS(SentryObjCSDK) flush:0];
	[SENTRY_APPLE_CLASS(SentryObjCSDK) close];
}

bool FAppleSentrySubsystem::IsEnabled()
{
	return [SENTRY_APPLE_CLASS(SentryObjCSDK) isEnabled];
}

ESentryCrashedLastRun FAppleSentrySubsystem::IsCrashedLastRun()
{
	return [SENTRY_APPLE_CLASS(SentryObjCSDK) crashedLastRun] ? ESentryCrashedLastRun::Crashed : ESentryCrashedLastRun::NotCrashed;
}

void FAppleSentrySubsystem::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	TSharedPtr<FAppleSentryBreadcrumb> breadcrumbIOS = StaticCastSharedPtr<FAppleSentryBreadcrumb>(breadcrumb);

	[SENTRY_APPLE_CLASS(SentryObjCSDK) addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void FAppleSentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level)
{
	TSharedPtr<FAppleSentryBreadcrumb> breadcrumbIOS = MakeShareable(new FAppleSentryBreadcrumb());
	breadcrumbIOS->SetMessage(Message);
	breadcrumbIOS->SetCategory(Category);
	breadcrumbIOS->SetType(Type);
	breadcrumbIOS->SetData(Data);
	breadcrumbIOS->SetLevel(Level);

	[SENTRY_APPLE_CLASS(SentryObjCSDK) addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void FAppleSentrySubsystem::AddLog(const FString& Message, ESentryLevel Level, const TMap<FString, FSentryVariant>& Attributes)
{
	NSMutableDictionary* attributesDict = [NSMutableDictionary dictionaryWithCapacity:Attributes.Num()];

	for (const auto& pair : Attributes)
	{
		SentryObjCAttribute* attribute = FAppleSentryConverters::VariantToAttributeNative(pair.Value);
		if (attribute != nil)
		{
			[attributesDict setObject:attribute.value forKey:pair.Key.GetNSString()];
		}
	}

	// Use level-specific Apple Sentry SDK logging functions with attributes
	switch (Level)
	{
	case ESentryLevel::Fatal:
		[[SENTRY_APPLE_CLASS(SentryObjCSDK) logger] fatal:Message.GetNSString() attributes:attributesDict];
		break;
	case ESentryLevel::Error:
		[[SENTRY_APPLE_CLASS(SentryObjCSDK) logger] error:Message.GetNSString() attributes:attributesDict];
		break;
	case ESentryLevel::Warning:
		[[SENTRY_APPLE_CLASS(SentryObjCSDK) logger] warn:Message.GetNSString() attributes:attributesDict];
		break;
	case ESentryLevel::Info:
		[[SENTRY_APPLE_CLASS(SentryObjCSDK) logger] info:Message.GetNSString() attributes:attributesDict];
		break;
	case ESentryLevel::Debug:
	default:
		[[SENTRY_APPLE_CLASS(SentryObjCSDK) logger] debug:Message.GetNSString() attributes:attributesDict];
		break;
	}
}

void FAppleSentrySubsystem::AddCount(const FString& Key, int32 Value, const TMap<FString, FSentryVariant>& Attributes)
{
	NSDictionary<NSString*, SentryObjCAttributeContent*>* attributesDict = FAppleSentryConverters::VariantMapToAttributeContentNative(Attributes);

	[[SENTRY_APPLE_CLASS(SentryObjCSDK) metrics] countWithKey:Key.GetNSString()
														value:(NSUInteger)Value
												   attributes:attributesDict];
}

void FAppleSentrySubsystem::AddDistribution(const FString& Key, float Value, const FString& Unit, const TMap<FString, FSentryVariant>& Attributes)
{
	NSDictionary<NSString*, SentryObjCAttributeContent*>* attributesDict = FAppleSentryConverters::VariantMapToAttributeContentNative(Attributes);
	SentryObjCUnit* effectiveUnit = Unit.IsEmpty() ? nil : [[SENTRY_APPLE_CLASS(SentryObjCUnit) alloc] initWithRawValue:Unit.GetNSString()];

	[[SENTRY_APPLE_CLASS(SentryObjCSDK) metrics] distributionWithKey:Key.GetNSString()
															   value:(double)Value
																unit:effectiveUnit
														  attributes:attributesDict];
}

void FAppleSentrySubsystem::AddGauge(const FString& Key, float Value, const FString& Unit, const TMap<FString, FSentryVariant>& Attributes)
{
	NSDictionary<NSString*, SentryObjCAttributeContent*>* attributesDict = FAppleSentryConverters::VariantMapToAttributeContentNative(Attributes);
	SentryObjCUnit* effectiveUnit = Unit.IsEmpty() ? nil : [[SENTRY_APPLE_CLASS(SentryObjCUnit) alloc] initWithRawValue:Unit.GetNSString()];

	[[SENTRY_APPLE_CLASS(SentryObjCSDK) metrics] gaugeWithKey:Key.GetNSString()
														value:(double)Value
														 unit:effectiveUnit
												   attributes:attributesDict];
}

void FAppleSentrySubsystem::ClearBreadcrumbs()
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		[scope clearBreadcrumbs];
	}];
}

void FAppleSentrySubsystem::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FAppleSentryAttachment> attachmentApple = StaticCastSharedPtr<FAppleSentryAttachment>(attachment);

	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		[scope addAttachment:attachmentApple->GetNativeObject()];
	}];
}

void FAppleSentrySubsystem::RemoveAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	// Currently, Cocoa SDK doesn't have API allowing to remove individual attachments
}

void FAppleSentrySubsystem::ClearAttachments()
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		[scope clearAttachments];
	}];
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureMessage(const FString& message, ESentryLevel level)
{
	FSentryScopeDelegate onConfigureScope;
	return CaptureMessageWithScope(message, level, onConfigureScope);
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureMessageWithScope(const FString& message, ESentryLevel level, const FSentryScopeDelegate& onConfigureScope)
{
	SentryObjCId* nativeId = [SENTRY_APPLE_CLASS(SentryObjCSDK) captureMessage:message.GetNSString() withScopeBlock:^(SentryObjCScope* scope) {
		[scope setLevel:FAppleSentryConverters::SentryLevelToNative(level)];
		onConfigureScope.ExecuteIfBound(MakeShareable(new FAppleSentryScope(scope)));
	}];

	TSharedPtr<ISentryId> id = MakeShareable(new FAppleSentryId(nativeId));

	if (isGameLogAttachmentEnabled)
	{
		UploadGameLogForEvent(id, GetGameLogPath());
	}

	return id;
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureEvent(TSharedPtr<ISentryEvent> event)
{
	FSentryScopeDelegate onConfigureScope;
	return CaptureEventWithScope(event, onConfigureScope);
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onConfigureScope)
{
	TSharedPtr<FAppleSentryEvent> eventApple = StaticCastSharedPtr<FAppleSentryEvent>(event);

	SentryObjCId* nativeId = [SENTRY_APPLE_CLASS(SentryObjCSDK) captureEvent:eventApple->GetNativeObject() withScopeBlock:^(SentryObjCScope* scope) {
		onConfigureScope.ExecuteIfBound(MakeShareable(new FAppleSentryScope(scope)));
	}];

	TSharedPtr<ISentryId> id = MakeShareable(new FAppleSentryId(nativeId));

	if (isGameLogAttachmentEnabled)
	{
		UploadGameLogForEvent(id, GetGameLogPath());
	}

	return id;
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureEnsure(const FString& type, const FString& message)
{
	SentryObjCException* nativeException = [[SENTRY_APPLE_CLASS(SentryObjCException) alloc] initWithValue:message.GetNSString() type:type.GetNSString()];
	NSMutableArray* nativeExceptionArray = [NSMutableArray arrayWithCapacity:1];
	[nativeExceptionArray addObject:nativeException];

	SentryObjCEvent* exceptionEvent = [[SENTRY_APPLE_CLASS(SentryObjCEvent) alloc] init];
	exceptionEvent.exceptions = nativeExceptionArray;

	SentryObjCId* nativeId = [SENTRY_APPLE_CLASS(SentryObjCSDK) captureEvent:exceptionEvent];

	TSharedPtr<ISentryId> id = MakeShareable(new FAppleSentryId(nativeId));

	if (isGameLogAttachmentEnabled)
	{
		UploadGameLogForEvent(id, GetGameLogPath());
	}

	return id;
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureHang(uint32 HungThreadId)
{
	// Hang tracking is handled by the native Apple SDK via built-in App Hang detection (see EnableAppNotRespondingTracking setting)
	return nullptr;
}

bool FAppleSentrySubsystem::IsHangTrackingSupported() const
{
	return false;
}

bool FAppleSentrySubsystem::IsNativeHangTrackingEnabled() const
{
	return false;
}

void FAppleSentrySubsystem::CaptureFeedback(TSharedPtr<ISentryFeedback> feedback)
{
	TSharedPtr<FAppleSentryFeedback> feedbackApple = StaticCastSharedPtr<FAppleSentryFeedback>(feedback);

	SentryObjCId* associatedEventId = nil;
	if (!feedbackApple->GetAssociatedEvent().IsEmpty())
	{
		TSharedPtr<FAppleSentryId> idApple = MakeShareable(new FAppleSentryId(feedbackApple->GetAssociatedEvent()));
		associatedEventId = idApple->GetNativeObject();
	}

	NSMutableArray<SentryObjCAttachment*>* attachments = nil;
	const TArray<TSharedPtr<ISentryAttachment>>& feedbackAttachments = feedbackApple->GetAttachments();
	if (feedbackAttachments.Num() > 0)
	{
		attachments = [NSMutableArray arrayWithCapacity:feedbackAttachments.Num()];
		for (const TSharedPtr<ISentryAttachment>& attachment : feedbackAttachments)
		{
			TSharedPtr<FAppleSentryAttachment> attachmentApple = StaticCastSharedPtr<FAppleSentryAttachment>(attachment);
			if (attachmentApple)
			{
				SentryObjCAttachment* nativeAttachment = attachmentApple->GetNativeObject();
				if (nativeAttachment != nil)
				{
					[attachments addObject:nativeAttachment];
				}
			}
		}
	}

	[SENTRY_APPLE_CLASS(SentryObjCSDK) captureFeedbackWithMessage:feedbackApple->GetMessage().GetNSString()
															 name:feedbackApple->GetName().IsEmpty() ? nil : feedbackApple->GetName().GetNSString()
															email:feedbackApple->GetContactEmail().IsEmpty() ? nil : feedbackApple->GetContactEmail().GetNSString()
														   source:SentryObjCFeedbackSourceCustom
												associatedEventId:associatedEventId
													  attachments:attachments];
}

void FAppleSentrySubsystem::SetUser(TSharedPtr<ISentryUser> user)
{
	TSharedPtr<FAppleSentryUser> userIOS = StaticCastSharedPtr<FAppleSentryUser>(user);

	[SENTRY_APPLE_CLASS(SentryObjCSDK) setUser:userIOS->GetNativeObject()];
}

void FAppleSentrySubsystem::RemoveUser()
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) setUser:nil];
}

void FAppleSentrySubsystem::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		[scope setContextValue:FAppleSentryConverters::VariantMapToNative(values) forKey:key.GetNSString()];
	}];
}

void FAppleSentrySubsystem::SetTag(const FString& key, const FString& value)
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		[scope setTagValue:value.GetNSString() forKey:key.GetNSString()];
	}];
}

void FAppleSentrySubsystem::RemoveTag(const FString& key)
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		[scope removeTagForKey:key.GetNSString()];
	}];
}

void FAppleSentrySubsystem::SetAttribute(const FString& key, const FSentryVariant& value)
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		id attrValue = FAppleSentryConverters::VariantToNative(value);
		[scope setAttributeValue:attrValue forKey:key.GetNSString()];
	}];
}

void FAppleSentrySubsystem::RemoveAttribute(const FString& key)
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		[scope removeAttributeForKey:key.GetNSString()];
	}];
}

void FAppleSentrySubsystem::SetLevel(ESentryLevel level)
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		[scope setLevel:FAppleSentryConverters::SentryLevelToNative(level)];
	}];
}

void FAppleSentrySubsystem::SetRelease(const FString& release)
{
	// sentry-cocoa does not provide a public API to change release after initialization
}

void FAppleSentrySubsystem::SetEnvironment(const FString& environment)
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) configureScope:^(SentryObjCScope* scope) {
		[scope setEnvironment:environment.GetNSString()];
	}];
}

void FAppleSentrySubsystem::StartSession()
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) startSession];
}

void FAppleSentrySubsystem::EndSession()
{
	[SENTRY_APPLE_CLASS(SentryObjCSDK) endSession];
}

void FAppleSentrySubsystem::GiveUserConsent()
{
	// No-op; feature not currently implemented for this platform
	UE_LOG(LogSentrySdk, Log, TEXT("GiveUserConsent is not supported on Mac/iOS."));
}

void FAppleSentrySubsystem::RevokeUserConsent()
{
	// No-op; feature not currently implemented for this platform
	UE_LOG(LogSentrySdk, Log, TEXT("RevokeUserConsent is not supported on Mac/iOS."));
}

EUserConsent FAppleSentrySubsystem::GetUserConsent() const
{
	UE_LOG(LogSentrySdk, Log, TEXT("GetUserConsent is not supported on Mac/iOS. Returning default `Unknown` value."));
	return EUserConsent::Unknown;
}

bool FAppleSentrySubsystem::IsUserConsentRequired() const
{
	UE_LOG(LogSentrySdk, Log, TEXT("IsUserConsentRequired is not supported on Mac/iOS. Returning default `false` value."));
	return false;
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransaction(const FString& name, const FString& operation, bool bindToScope)
{
	SentryObjCSpan* transaction = [SENTRY_APPLE_CLASS(SentryObjCSDK) startTransactionWithName:name.GetNSString() operation:operation.GetNSString() bindToScope:bindToScope];

	return MakeShareable(new FAppleSentryTransaction(transaction));
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context, bool bindToScope)
{
	TSharedPtr<FAppleSentryTransactionContext> transactionContextIOS = StaticCastSharedPtr<FAppleSentryTransactionContext>(context);

	SentryObjCSpan* transaction = [SENTRY_APPLE_CLASS(SentryObjCSDK) startTransactionWithContext:transactionContextIOS->GetNativeObject() bindToScope:bindToScope];

	return MakeShareable(new FAppleSentryTransaction(transaction));
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp, bool bindToScope)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Setting transaction timestamp explicitly not supported on Mac/iOS."));
	return StartTransactionWithContext(context, bindToScope);
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const FSentryTransactionOptions& options)
{
	TSharedPtr<FAppleSentryTransactionContext> transactionContextIOS = StaticCastSharedPtr<FAppleSentryTransactionContext>(context);

	SentryObjCSpan* transaction = [SENTRY_APPLE_CLASS(SentryObjCSDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()
																					 bindToScope:options.BindToScope
																		   customSamplingContext:FAppleSentryConverters::VariantMapToNative(options.CustomSamplingContext)];

	return MakeShareable(new FAppleSentryTransaction(transaction));
}

TSharedPtr<ISentryTransactionContext> FAppleSentrySubsystem::ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders)
{
	TArray<FString> traceParts;
	sentryTrace.ParseIntoArray(traceParts, TEXT("-"));

	if (traceParts.Num() < 2)
	{
		return nullptr;
	}

	SentryObjCSampleDecision sampleDecision = SentryObjCSampleDecisionUndecided;
	if (traceParts.Num() == 3)
	{
		sampleDecision = traceParts[2].Equals(TEXT("1")) ? SentryObjCSampleDecisionYes : SentryObjCSampleDecisionNo;
	}

	SentryObjCId* traceId = [[SENTRY_APPLE_CLASS(SentryObjCId) alloc] initWithUUIDString:traceParts[0].GetNSString()];

	SentryObjCTransactionContext* transactionContext = [[SENTRY_APPLE_CLASS(SentryObjCTransactionContext) alloc] initWithName:@"<unlabeled transaction>" operation:@"default"
																													  traceId:traceId
																													   spanId:[[SENTRY_APPLE_CLASS(SentryObjCSpanId) alloc] init]
																												 parentSpanId:[[SENTRY_APPLE_CLASS(SentryObjCSpanId) alloc] initWithValue:traceParts[1].GetNSString()]
																												parentSampled:sampleDecision
																											 parentSampleRate:nil
																											 parentSampleRand:nil];

	// currently `sentry-cocoa` doesn't have API for `SentryTransactionContext` to set `baggageHeaders`

	return MakeShareable(new FAppleSentryTransactionContext(transactionContext));
}

void FAppleSentrySubsystem::UploadAttachmentForEvent(TSharedPtr<ISentryId> eventId, const FString& filePath, const FString& name, bool deleteAfterUpload) const
{
	IFileManager& fileManager = IFileManager::Get();
	if (!fileManager.FileExists(*filePath))
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to upload attachment - file path provided did not exist: %s"), *filePath);
		return;
	}

	const FString& filePathExt = fileManager.ConvertToAbsolutePathForExternalAppForRead(*filePath);

	SentryObjCAttachment* attachment = [[SENTRY_APPLE_CLASS(SentryObjCAttachment) alloc] initWithPath:filePathExt.GetNSString() filename:name.GetNSString()];

	SentryObjCEnvelopeItem* envelopeItem = [[SENTRY_APPLE_CLASS(SentryObjCEnvelopeItem) alloc] initWithAttachment:attachment maxAttachmentSize:maxAttachmentSize];
	if (envelopeItem == nil)
	{
		UE_LOG(LogSentrySdk, Error, TEXT("Failed to upload attachment - file exceeds max attachment size or could not be read: %s"), *filePath);
		return;
	}

	SentryObjCId* id = StaticCastSharedPtr<FAppleSentryId>(eventId)->GetNativeObject();

	SentryObjCEnvelopeHeader* envelopeHeader = [[SENTRY_APPLE_CLASS(SentryObjCEnvelopeHeader) alloc] initWithId:id traceContext:nil];

	SentryObjCEnvelope* envelope = [[SENTRY_APPLE_CLASS(SentryObjCEnvelope) alloc] initWithHeader:envelopeHeader singleItem:envelopeItem];

	[[[SENTRY_APPLE_CLASS(SentryObjCSDK) internal] envelope] capture:envelope];

	if (deleteAfterUpload)
	{
		if (!fileManager.Delete(*filePath))
		{
			UE_LOG(LogSentrySdk, Error, TEXT("Failed to delete file attachment after upload: %s"), *filePath);
		}
	}
}

void FAppleSentrySubsystem::UploadScreenshotForEvent(TSharedPtr<ISentryId> eventId, const FString& screenshotPath) const
{
	if (screenshotPath.IsEmpty())
	{
		// Screenshot capturing is a best-effort solution so if one wasn't captured (path is empty) skip the upload
		return;
	}

	UploadAttachmentForEvent(eventId, screenshotPath, TEXT("screenshot.png"), true);
}

void FAppleSentrySubsystem::UploadGameLogForEvent(TSharedPtr<ISentryId> eventId, const FString& logFilePath) const
{
	// If writing logs to a file is disabled (i.e. default behavior for Shipping builds) skip the upload
#if !NO_LOGGING
	UploadAttachmentForEvent(eventId, logFilePath, SentryFileUtils::GetGameLogName());
#endif
}

void FAppleSentrySubsystem::UploadSessionReplayForEvent(TSharedPtr<ISentryId> eventId, const FString& replayPath) const
{
	if (replayPath.IsEmpty())
	{
		// Recorder only produces a file after the first keyframe so if a crash happens early and nothing is written to disk (empty path) skip the upload
		return;
	}

	UploadAttachmentForEvent(eventId, replayPath, TEXT("session-replay.mp4"), false);
}

FString FAppleSentrySubsystem::GetScreenshotPath() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SentryScreenshots"), FString::Printf(TEXT("screenshot-%s.png"), *FDateTime::Now().ToString()));
}

FString FAppleSentrySubsystem::GetLatestScreenshot() const
{
	const FString& ScreenshotsDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SentryScreenshots"));

	TArray<FString> Screenshots;
	IFileManager::Get().FindFiles(Screenshots, *ScreenshotsDir, TEXT("*.png"));

	if (Screenshots.Num() == 0)
	{
		UE_LOG(LogSentrySdk, Log, TEXT("There are no screenshots found."));
		return FString("");
	}

	for (int i = 0; i < Screenshots.Num(); ++i)
	{
		Screenshots[i] = ScreenshotsDir / Screenshots[i];
	}

	Screenshots.Sort([](const FString& A, const FString& B)
	{
		const FDateTime TimestampA = IFileManager::Get().GetTimeStamp(*A);
		const FDateTime TimestampB = IFileManager::Get().GetTimeStamp(*B);
		return TimestampB < TimestampA;
	});

	return Screenshots[0];
}

bool FAppleSentrySubsystem::GetLatestSessionReplay(FString& OutReplayPath, FString& OutSidecarPath) const
{
	const FString& ReplaysDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SentryReplays"));

	TArray<FString> Replays;
	IFileManager::Get().FindFiles(Replays, *ReplaysDir, TEXT("*.mp4"));

	if (Replays.Num() == 0)
	{
		return false;
	}

	for (int i = 0; i < Replays.Num(); ++i)
	{
		Replays[i] = ReplaysDir / Replays[i];
	}

	Replays.Sort([](const FString& A, const FString& B)
	{
		const FDateTime TimestampA = IFileManager::Get().GetTimeStamp(*A);
		const FDateTime TimestampB = IFileManager::Get().GetTimeStamp(*B);
		return TimestampB < TimestampA;
	});

	const FString& LatestReplay = Replays[0];

	// The metadata sidecar is written alongside the video file and shares its name
	const FString LatestSidecar = FPaths::ChangeExtension(LatestReplay, TEXT("json"));
	if (!FPaths::FileExists(LatestSidecar))
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("There is no metadata sidecar accompanying the latest session replay."));
		return false;
	}

	OutReplayPath = LatestReplay;
	OutSidecarPath = LatestSidecar;

	return true;
}

#ifdef USE_SENTRY_SESSION_REPLAY
FString FAppleSentrySubsystem::GetReplayPath() const
{
	const FString ReplayPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SentryReplays"), FString::Printf(TEXT("replay-%s.mp4"), *SessionReplayId));
	return FPaths::ConvertRelativePathToFull(ReplayPath);
}
#endif

#endif // !USE_SENTRY_NATIVE
