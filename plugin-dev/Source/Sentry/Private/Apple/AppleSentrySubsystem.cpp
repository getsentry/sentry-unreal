// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentrySubsystem.h"

#include "AppleSentryAttachment.h"
#include "AppleSentryBreadcrumb.h"
#include "AppleSentryEvent.h"
#include "AppleSentryFeedback.h"
#include "AppleSentryId.h"
#include "AppleSentryLog.h"
#include "AppleSentrySamplingContext.h"
#include "AppleSentryScope.h"
#include "AppleSentryTransaction.h"
#include "AppleSentryTransactionContext.h"
#include "AppleSentryUser.h"

#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryBeforeLogHandler.h"
#include "SentryBeforeSendHandler.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryEvent.h"
#include "SentryLog.h"
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

void FAppleSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryBeforeLogHandler* beforeLogHandler, USentryTraceSampler* traceSampler)
{
	isScreenshotAttachmentEnabled = settings->AttachScreenshot;
	isGameLogAttachmentEnabled = settings->EnableAutoLogAttachment;

	[SENTRY_APPLE_CLASS(PrivateSentrySDKOnly) setSdkName:@"sentry.cocoa.unreal"];

	dispatch_group_t sentryDispatchGroup = dispatch_group_create();
	dispatch_group_enter(sentryDispatchGroup);
	dispatch_async(dispatch_get_main_queue(), ^{
		[SENTRY_APPLE_CLASS(SentrySDK) startWithConfigureOptions:^(SentryOptions* options) {
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
			options.experimental.enableLogs = settings->EnableStructuredLogging;
#if SENTRY_UIKIT_AVAILABLE
			options.attachScreenshot = settings->AttachScreenshot;
#endif
			options.onCrashedLastRun = ^(SentryEvent* event) {
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
			};
			for (auto it = settings->InAppInclude.CreateConstIterator(); it; ++it)
			{
				[options addInAppInclude:it->GetNSString()];
			}
			for (auto it = settings->InAppExclude.CreateConstIterator(); it; ++it)
			{
				[options addInAppExclude:it->GetNSString()];
			}
			options.enableAppHangTracking = settings->EnableAppNotRespondingTracking;
			if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::UniformSampleRate)
			{
				options.tracesSampleRate = [NSNumber numberWithFloat:settings->TracesSampleRate];
			}
			if (settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::TracesSampler && traceSampler != nullptr)
			{
				options.tracesSampler = ^NSNumber*(SentrySamplingContext* samplingContext) {
					if (!SentryCallbackUtils::IsCallbackSafeToRun())
					{
						// Falling back to default sampling value without calling a custom sampling function
						return nil;
					}

					USentrySamplingContext* Context = USentrySamplingContext::Create(MakeShareable(new FAppleSentrySamplingContext(samplingContext)));

					float samplingValue;
					return traceSampler->Sample(Context, samplingValue) ? [NSNumber numberWithFloat:samplingValue] : nil;
				};
			}
			if (beforeBreadcrumbHandler != nullptr)
			{
				options.beforeBreadcrumb = ^SentryBreadcrumb*(SentryBreadcrumb* breadcrumb) {
					if (!SentryCallbackUtils::IsCallbackSafeToRun())
					{
						// Breadcrumb will be added without calling a `beforeBreadcrumb` handler
						return breadcrumb;
					}

					USentryBreadcrumb* BreadcrumbToProcess = USentryBreadcrumb::Create(MakeShareable(new FAppleSentryBreadcrumb(breadcrumb)));

					USentryBreadcrumb* ProcessedBreadcrumb = beforeBreadcrumbHandler->HandleBeforeBreadcrumb(BreadcrumbToProcess, nullptr);

					return ProcessedBreadcrumb ? breadcrumb : nullptr;
				};
			}
			if (beforeLogHandler != nullptr)
			{
				options.beforeSendLog = ^SentryLog*(SentryLog* log) {
					if (!SentryCallbackUtils::IsCallbackSafeToRun())
					{
						// Log will be added without calling a `onBeforeLog` handler
						return log;
					}

					USentryLog* LogToProcess = USentryLog::Create(MakeShareable(new FAppleSentryLog(log)));

					USentryLog* ProcessedLog = beforeLogHandler->HandleBeforeLog(LogToProcess);

					return ProcessedLog ? log : nullptr;
				};
			}
			if (beforeSendHandler != nullptr)
			{
				options.beforeSend = ^SentryEvent*(SentryEvent* event) {
					if (!SentryCallbackUtils::IsCallbackSafeToRun())
					{
						// Event will be sent without calling a `onBeforeSend` handler
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
}

void FAppleSentrySubsystem::Close()
{
	[SENTRY_APPLE_CLASS(SentrySDK) close];
}

bool FAppleSentrySubsystem::IsEnabled()
{
	return [SENTRY_APPLE_CLASS(SentrySDK) isEnabled];
}

ESentryCrashedLastRun FAppleSentrySubsystem::IsCrashedLastRun()
{
	return [SENTRY_APPLE_CLASS(SentrySDK) crashedLastRun] ? ESentryCrashedLastRun::Crashed : ESentryCrashedLastRun::NotCrashed;
}

void FAppleSentrySubsystem::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	TSharedPtr<FAppleSentryBreadcrumb> breadcrumbIOS = StaticCastSharedPtr<FAppleSentryBreadcrumb>(breadcrumb);

	[SENTRY_APPLE_CLASS(SentrySDK) addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void FAppleSentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data, ESentryLevel Level)
{
	TSharedPtr<FAppleSentryBreadcrumb> breadcrumbIOS = MakeShareable(new FAppleSentryBreadcrumb());
	breadcrumbIOS->SetMessage(Message);
	breadcrumbIOS->SetCategory(Category);
	breadcrumbIOS->SetType(Type);
	breadcrumbIOS->SetData(Data);
	breadcrumbIOS->SetLevel(Level);

	[SENTRY_APPLE_CLASS(SentrySDK) addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void FAppleSentrySubsystem::AddLog(const FString& Body, ESentryLevel Level, const FString& Category)
{
	// Ignore Empty Bodies
	if (Body.IsEmpty())
	{
		return;
	}

	// Format body with category
	NSString* FormattedMessage;
	if (!Category.IsEmpty())
	{
		FString FullMessage = FString::Printf(TEXT("[%s] %s"), *Category, *Body);
		FormattedMessage = FullMessage.GetNSString();
	}
	else
	{
		FormattedMessage = Body.GetNSString();
	}

	// Use level-specific Apple Sentry SDK logging functions
	switch (Level)
	{
	case ESentryLevel::Fatal:
		[[SENTRY_APPLE_CLASS(SentrySDK) logger] fatal:FormattedMessage];
		break;
	case ESentryLevel::Error:
		[[SENTRY_APPLE_CLASS(SentrySDK) logger] error:FormattedMessage];
		break;
	case ESentryLevel::Warning:
		[[SENTRY_APPLE_CLASS(SentrySDK) logger] warn:FormattedMessage];
		break;
	case ESentryLevel::Info:
		[[SENTRY_APPLE_CLASS(SentrySDK) logger] info:FormattedMessage];
		break;
	case ESentryLevel::Debug:
	default:
		[[SENTRY_APPLE_CLASS(SentrySDK) logger] debug:FormattedMessage];
		break;
	}
}

void FAppleSentrySubsystem::ClearBreadcrumbs()
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope clearBreadcrumbs];
	}];
}

void FAppleSentrySubsystem::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FAppleSentryAttachment> attachmentApple = StaticCastSharedPtr<FAppleSentryAttachment>(attachment);

	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope addAttachment:attachmentApple->GetNativeObject()];
	}];
}

void FAppleSentrySubsystem::RemoveAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	// Currently, Cocoa SDK doesn't have API allowing to remove individual attachments
}

void FAppleSentrySubsystem::ClearAttachments()
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
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
	SentryId* nativeId = [SENTRY_APPLE_CLASS(SentrySDK) captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope) {
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

	SentryId* nativeId = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:eventApple->GetNativeObject() withScopeBlock:^(SentryScope* scope) {
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
	SentryException* nativeException = [[SENTRY_APPLE_CLASS(SentryException) alloc] initWithValue:message.GetNSString() type:type.GetNSString()];
	NSMutableArray* nativeExceptionArray = [NSMutableArray arrayWithCapacity:1];
	[nativeExceptionArray addObject:nativeException];

	SentryEvent* exceptionEvent = [[SENTRY_APPLE_CLASS(SentryEvent) alloc] init];
	exceptionEvent.exceptions = nativeExceptionArray;

	SentryId* nativeId = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:exceptionEvent];

	TSharedPtr<ISentryId> id = MakeShareable(new FAppleSentryId(nativeId));

	if (isGameLogAttachmentEnabled)
	{
		UploadGameLogForEvent(id, GetGameLogPath());
	}

	return id;
}

void FAppleSentrySubsystem::CaptureFeedback(TSharedPtr<ISentryFeedback> feedback)
{
	TSharedPtr<FAppleSentryFeedback> feedbackApple = StaticCastSharedPtr<FAppleSentryFeedback>(feedback);

	[SENTRY_APPLE_CLASS(SentrySDK) captureFeedback:FAppleSentryFeedback::CreateSentryFeedback(feedbackApple)];
}

void FAppleSentrySubsystem::SetUser(TSharedPtr<ISentryUser> user)
{
	TSharedPtr<FAppleSentryUser> userIOS = StaticCastSharedPtr<FAppleSentryUser>(user);

	[SENTRY_APPLE_CLASS(SentrySDK) setUser:userIOS->GetNativeObject()];
}

void FAppleSentrySubsystem::RemoveUser()
{
	[SENTRY_APPLE_CLASS(SentrySDK) setUser:nil];
}

void FAppleSentrySubsystem::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope setContextValue:FAppleSentryConverters::VariantMapToNative(values) forKey:key.GetNSString()];
	}];
}

void FAppleSentrySubsystem::SetTag(const FString& key, const FString& value)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope setTagValue:value.GetNSString() forKey:key.GetNSString()];
	}];
}

void FAppleSentrySubsystem::RemoveTag(const FString& key)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope removeTagForKey:key.GetNSString()];
	}];
}

void FAppleSentrySubsystem::SetLevel(ESentryLevel level)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope setLevel:FAppleSentryConverters::SentryLevelToNative(level)];
	}];
}

void FAppleSentrySubsystem::StartSession()
{
	[SENTRY_APPLE_CLASS(SentrySDK) startSession];
}

void FAppleSentrySubsystem::EndSession()
{
	[SENTRY_APPLE_CLASS(SentrySDK) endSession];
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

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransaction(const FString& name, const FString& operation, bool bindToScope)
{
	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithName:name.GetNSString() operation:operation.GetNSString() bindToScope:bindToScope];

	return MakeShareable(new FAppleSentryTransaction(transaction));
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context, bool bindToScope)
{
	TSharedPtr<FAppleSentryTransactionContext> transactionContextIOS = StaticCastSharedPtr<FAppleSentryTransactionContext>(context);

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject() bindToScope:bindToScope];

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

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()
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

	SentrySampleDecision sampleDecision = kSentrySampleDecisionUndecided;
	if (traceParts.Num() == 3)
	{
		sampleDecision = traceParts[2].Equals(TEXT("1")) ? kSentrySampleDecisionYes : kSentrySampleDecisionNo;
	}

	SentryId* traceId = [[SENTRY_APPLE_CLASS(SentryId) alloc] initWithUUIDString:traceParts[0].GetNSString()];

	SentryTransactionContext* transactionContext = [[SENTRY_APPLE_CLASS(SentryTransactionContext) alloc] initWithName:@"<unlabeled transaction>" operation:@"default"
																											  traceId:traceId
																											   spanId:[[SENTRY_APPLE_CLASS(SentrySpanId) alloc] init]
																										 parentSpanId:[[SENTRY_APPLE_CLASS(SentrySpanId) alloc] initWithValue:traceParts[1].GetNSString()]
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

	SentryAttachment* attachment = [[SENTRY_APPLE_CLASS(SentryAttachment) alloc] initWithPath:filePathExt.GetNSString() filename:name.GetNSString()];

	SentryOptions* options = [SENTRY_APPLE_CLASS(PrivateSentrySDKOnly) options];
	int32 size = options.maxAttachmentSize;

	SentryEnvelopeItem* envelopeItem = [[SENTRY_APPLE_CLASS(SentryEnvelopeItem) alloc] initWithAttachment:attachment maxAttachmentSize:size];

	SentryId* id = StaticCastSharedPtr<FAppleSentryId>(eventId)->GetNativeObject();

	SentryEnvelopeHeader* envelopeHeader = [[SENTRY_APPLE_CLASS(SentryEnvelopeHeader) alloc] initWithId:id sdkInfo:nil traceContext:nil];

	SentryEnvelope* envelope = [[SENTRY_APPLE_CLASS(SentryEnvelope) alloc] initWithHeader:envelopeHeader singleItem:envelopeItem];

	[SENTRY_APPLE_CLASS(PrivateSentrySDKOnly) captureEnvelope:envelope];

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
#if NO_LOGGING
	// If writing logs to a file is disabled (i.e. default behavior for Shipping builds) skip the upload
	return;
#endif

	UploadAttachmentForEvent(eventId, logFilePath, SentryFileUtils::GetGameLogName());
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
