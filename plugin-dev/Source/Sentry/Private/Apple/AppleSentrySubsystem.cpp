// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentrySubsystem.h"

#include "AppleSentryAttachment.h"
#include "AppleSentryBreadcrumb.h"
#include "AppleSentryEvent.h"
#include "AppleSentryId.h"
#include "AppleSentrySamplingContext.h"
#include "AppleSentryScope.h"
#include "AppleSentryTransaction.h"
#include "AppleSentryTransactionContext.h"
#include "AppleSentryUser.h"
#include "AppleSentryUserFeedback.h"
#include "Convenience/AppleSentryMacro.h"

#include "SentryBeforeBreadcrumbHandler.h"
#include "SentryBeforeSendHandler.h"
#include "SentryBreadcrumb.h"
#include "SentryDefines.h"
#include "SentryEvent.h"
#include "SentrySamplingContext.h"
#include "SentrySettings.h"
#include "SentryTraceSampler.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"

#include "Utils/SentryFileUtils.h"
#include "Utils/SentryLogUtils.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformSentryAttachment.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"

void FAppleSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryBeforeBreadcrumbHandler* beforeBreadcrumbHandler, USentryTraceSampler* traceSampler)
{
	isScreenshotAttachmentEnabled = settings->AttachScreenshot;
	isGameLogAttachmentEnabled = settings->EnableAutoLogAttachment;

	[SENTRY_APPLE_CLASS(PrivateSentrySDKOnly) setSdkName:@"sentry.cocoa.unreal"];

	dispatch_group_t sentryDispatchGroup = dispatch_group_create();
	dispatch_group_enter(sentryDispatchGroup);
	dispatch_async(dispatch_get_main_queue(), ^{
		[SENTRY_APPLE_CLASS(SentrySDK) startWithConfigureOptions:^(SentryOptions* options) {
			options.dsn = settings->GetEffectiveDsn().GetNSString();
			options.environment = settings->Environment.GetNSString();
			options.dist = settings->Dist.GetNSString();
			options.enableAutoSessionTracking = settings->EnableAutoSessionTracking;
			options.sessionTrackingIntervalMillis = settings->SessionTimeout;
			options.releaseName = settings->OverrideReleaseName ? settings->Release.GetNSString() : settings->GetFormattedReleaseName().GetNSString();
			options.attachStacktrace = settings->AttachStacktrace;
			options.debug = settings->Debug;
			options.sampleRate = [NSNumber numberWithFloat:settings->SampleRate];
			options.maxBreadcrumbs = settings->MaxBreadcrumbs;
			options.sendDefaultPii = settings->SendDefaultPii;
			options.maxAttachmentSize = settings->MaxAttachmentSize;
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
					FGCScopeGuard GCScopeGuard;
					USentrySamplingContext* Context = USentrySamplingContext::Create(MakeShareable(new FAppleSentrySamplingContext(samplingContext)));
					float samplingValue;
					return traceSampler->Sample(Context, samplingValue) ? [NSNumber numberWithFloat:samplingValue] : nil;
				};
			}
			if (beforeBreadcrumbHandler != nullptr)
			{
				options.beforeBreadcrumb = ^SentryBreadcrumb*(SentryBreadcrumb* breadcrumb) {
					if (FUObjectThreadContext::Get().IsRoutingPostLoad)
					{
						// Don't print to logs within `onBeforeBreadcrumb` handler as this can lead to creating new breadcrumb
						return breadcrumb;
					}

					if (IsGarbageCollecting())
					{
						// If breadcrumb is added during garbage collection we can't instantiate UObjects safely or obtain a GC lock
						// since there is no guarantee it will be ever freed.
						// In this case breadcrumb will be added without calling a `beforeBreadcrumb` handler.
						return breadcrumb;
					}

					USentryBreadcrumb* BreadcrumbToProcess = USentryBreadcrumb::Create(MakeShareable(new FAppleSentryBreadcrumb(breadcrumb)));

					USentryBreadcrumb* ProcessedBreadcrumb = beforeBreadcrumbHandler->HandleBeforeBreadcrumb(BreadcrumbToProcess, nullptr);

					return ProcessedBreadcrumb ? breadcrumb : nullptr;
				};
			}
			if (beforeSendHandler != nullptr)
			{
				options.beforeSend = ^SentryEvent*(SentryEvent* event) {
					if (FUObjectThreadContext::Get().IsRoutingPostLoad)
					{
						UE_LOG(LogSentrySdk, Log, TEXT("Executing `beforeSend` handler is not allowed during object post-loading."));
						return event;
					}

					if (IsGarbageCollecting())
					{
						// If event is captured during garbage collection we can't instantiate UObjects safely or obtain a GC lock
						// since it will cause a deadlock (see https://github.com/getsentry/sentry-unreal/issues/850).
						// In this case event will be reported without calling a `beforeSend` handler.
						UE_LOG(LogSentrySdk, Log, TEXT("Executing `beforeSend` handler is not allowed during garbage collection."));
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

void FAppleSentrySubsystem::CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback)
{
	TSharedPtr<FAppleSentryUserFeedback> userFeedbackApple = StaticCastSharedPtr<FAppleSentryUserFeedback>(userFeedback);

	[SENTRY_APPLE_CLASS(SentrySDK) captureFeedback:FAppleSentryUserFeedback::CreateSentryFeedback(userFeedbackApple)];
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

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransaction(const FString& name, const FString& operation)
{
	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithName:name.GetNSString() operation:operation.GetNSString()];

	return MakeShareable(new FAppleSentryTransaction(transaction));
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context)
{
	TSharedPtr<FAppleSentryTransactionContext> transactionContextIOS = StaticCastSharedPtr<FAppleSentryTransactionContext>(context);

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()];

	return MakeShareable(new FAppleSentryTransaction(transaction));
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Setting transaction timestamp explicitly not supported on Mac/iOS."));
	return StartTransactionWithContext(context);
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options)
{
	TSharedPtr<FAppleSentryTransactionContext> transactionContextIOS = StaticCastSharedPtr<FAppleSentryTransactionContext>(context);

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()
																	  customSamplingContext:FAppleSentryConverters::StringMapToNative(options)];

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

#if PLATFORM_MAC
	SentryId* traceId = [[SENTRY_APPLE_CLASS(_TtC6Sentry8SentryId) alloc] initWithUUIDString:traceParts[0].GetNSString()];
#elif PLATFORM_IOS
	SentryId* traceId = [[SENTRY_APPLE_CLASS(SentryId) alloc] initWithUUIDString:traceParts[0].GetNSString()];
#endif

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

	SentryEnvelope* envelope = [[SENTRY_APPLE_CLASS(SentryEnvelope) alloc] initWithId:id singleItem:envelopeItem];

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
