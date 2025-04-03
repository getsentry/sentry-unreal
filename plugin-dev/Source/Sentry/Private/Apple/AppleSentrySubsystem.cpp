// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "AppleSentrySubsystem.h"

#include "SentryBreadcrumbApple.h"
#include "SentryEventApple.h"
#include "SentryScopeApple.h"
#include "SentryUserApple.h"
#include "SentryUserFeedbackApple.h"
#include "SentryTransactionApple.h"
#include "SentryTransactionContextApple.h"
#include "SentrySamplingContextApple.h"
#include "SentryIdApple.h"

#include "SentryEvent.h"
#include "SentrySettings.h"
#include "SentryBeforeSendHandler.h"
#include "SentryDefines.h"
#include "SentrySamplingContext.h"
#include "SentryTraceSampler.h"

#include "Infrastructure/SentryConvertersApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/FileManager.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectThreadContext.h"
#include "Utils/SentryLogUtils.h"

void FAppleSentrySubsystem::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler)
{
	[SENTRY_APPLE_CLASS(PrivateSentrySDKOnly) setSdkName:@"sentry.cocoa.unreal"];

	dispatch_group_t sentryDispatchGroup = dispatch_group_create();
	dispatch_group_enter(sentryDispatchGroup);
	dispatch_async(dispatch_get_main_queue(), ^{
		[SENTRY_APPLE_CLASS(SentrySDK) startWithConfigureOptions:^(SentryOptions *options) {
			options.dsn = settings->Dsn.GetNSString();
#if WITH_EDITOR
			if(!settings->EditorDsn.IsEmpty()) {
				options.dsn = settings->EditorDsn.GetNSString();
			}
#endif
			options.environment = settings->Environment.GetNSString();
			options.enableAutoSessionTracking = settings->EnableAutoSessionTracking;
			options.sessionTrackingIntervalMillis = settings->SessionTimeout;
			options.releaseName = settings->OverrideReleaseName
				? settings->Release.GetNSString()
				: settings->GetFormattedReleaseName().GetNSString();
			options.attachStacktrace = settings->AttachStacktrace;
			options.debug = settings->Debug;
			options.sampleRate = [NSNumber numberWithFloat:settings->SampleRate];
			options.maxBreadcrumbs = settings->MaxBreadcrumbs;
			options.sendDefaultPii = settings->SendDefaultPii;
#if SENTRY_UIKIT_AVAILABLE
			options.attachScreenshot = settings->AttachScreenshot;
#endif
			options.initialScope = ^(SentryScope* scope) {
				if(settings->EnableAutoLogAttachment) {
					const FString logFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FGenericPlatformOutputDevices::GetAbsoluteLogFilename());
					SentryAttachment* logAttachment = [[SENTRY_APPLE_CLASS(SentryAttachment) alloc] initWithPath:logFilePath.GetNSString()];
					[scope addAttachment:logAttachment];
				}
				return scope;
			};
			options.beforeSend = ^SentryEvent* (SentryEvent* event) {
				if (FUObjectThreadContext::Get().IsRoutingPostLoad)
				{
					UE_LOG(LogSentrySdk, Log, TEXT("Executing `beforeSend` handler is not allowed when post-loading."));
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

				USentryEvent* EventToProcess = USentryEvent::Create(MakeShareable(new SentryEventApple(event)));

				USentryEvent* ProcessedEvent = beforeSendHandler->HandleBeforeSend(EventToProcess, nullptr);

				return ProcessedEvent ? event : nullptr;
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
			if(settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::UniformSampleRate)
			{
				options.tracesSampleRate = [NSNumber numberWithFloat:settings->TracesSampleRate];
			}
			if(settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::TracesSampler)
			{
				options.tracesSampler = ^NSNumber* (SentrySamplingContext* samplingContext) {
					FGCScopeGuard GCScopeGuard;
					USentrySamplingContext* Context = USentrySamplingContext::Create(MakeShareable(new SentrySamplingContextApple(samplingContext)));
					float samplingValue;
					return traceSampler->Sample(Context, samplingValue) ? [NSNumber numberWithFloat:samplingValue] : nil;
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
	TSharedPtr<SentryBreadcrumbApple> breadcrumbIOS = StaticCastSharedPtr<SentryBreadcrumbApple>(breadcrumb);

	[SENTRY_APPLE_CLASS(SentrySDK) addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void FAppleSentrySubsystem::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	TSharedPtr<SentryBreadcrumbApple> breadcrumbIOS = MakeShareable(new SentryBreadcrumbApple());
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

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureMessage(const FString& message, ESentryLevel level)
{
	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertersApple::SentryLevelToNative(level)];
	}];

	return MakeShareable(new SentryIdApple(id));
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureMessageWithScope(const FString& message, const FSentryScopeDelegate& onConfigureScope, ESentryLevel level)
{
	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertersApple::SentryLevelToNative(level)];
		onConfigureScope.ExecuteIfBound(MakeShareable(new SentryScopeApple(scope)));
	}];

	return MakeShareable(new SentryIdApple(id));
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureEvent(TSharedPtr<ISentryEvent> event)
{
	TSharedPtr<SentryEventApple> eventIOS = StaticCastSharedPtr<SentryEventApple>(event);

	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:eventIOS->GetNativeObject()];
	return MakeShareable(new SentryIdApple(id));
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onConfigureScope)
{
	TSharedPtr<SentryEventApple> eventIOS = StaticCastSharedPtr<SentryEventApple>(event);

	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:eventIOS->GetNativeObject() withScopeBlock:^(SentryScope* scope) {
		onConfigureScope.ExecuteIfBound(MakeShareable(new SentryScopeApple(scope)));
	}];

	return MakeShareable(new SentryIdApple(id));
}

TSharedPtr<ISentryId> FAppleSentrySubsystem::CaptureEnsure(const FString& type, const FString& message)
{
	SentryException *nativeException = [[SENTRY_APPLE_CLASS(SentryException) alloc] initWithValue:message.GetNSString() type:type.GetNSString()];
	NSMutableArray *nativeExceptionArray = [NSMutableArray arrayWithCapacity:1];
	[nativeExceptionArray addObject:nativeException];

	SentryEvent *exceptionEvent = [[SENTRY_APPLE_CLASS(SentryEvent) alloc] init];
	exceptionEvent.exceptions = nativeExceptionArray;

	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:exceptionEvent];
	return MakeShareable(new SentryIdApple(id));
}


void FAppleSentrySubsystem::CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback)
{
	TSharedPtr<SentryUserFeedbackApple> userFeedbackIOS = StaticCastSharedPtr<SentryUserFeedbackApple>(userFeedback);

	[SENTRY_APPLE_CLASS(SentrySDK) captureUserFeedback:userFeedbackIOS->GetNativeObject()];
}

void FAppleSentrySubsystem::SetUser(TSharedPtr<ISentryUser> user)
{
	TSharedPtr<SentryUserApple> userIOS = StaticCastSharedPtr<SentryUserApple>(user);

	[SENTRY_APPLE_CLASS(SentrySDK) setUser:userIOS->GetNativeObject()];
}

void FAppleSentrySubsystem::RemoveUser()
{
	[SENTRY_APPLE_CLASS(SentrySDK) setUser:nil];
}

void FAppleSentrySubsystem::ConfigureScope(const FSentryScopeDelegate& onConfigureScope)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		onConfigureScope.ExecuteIfBound(MakeShareable(new SentryScopeApple(scope)));
	}];
}

void FAppleSentrySubsystem::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope setContextValue:SentryConvertersApple::StringMapToNative(values) forKey:key.GetNSString()];
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
		[scope setLevel:SentryConvertersApple::SentryLevelToNative(level)];
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

	return MakeShareable(new SentryTransactionApple(transaction));
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context)
{
	TSharedPtr<SentryTransactionContextApple> transactionContextIOS = StaticCastSharedPtr<SentryTransactionContextApple>(context);

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()];

	return MakeShareable(new SentryTransactionApple(transaction));
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Setting transaction timestamp explicitly not supported on Mac/iOS."));
	return StartTransactionWithContext(context);
}

TSharedPtr<ISentryTransaction> FAppleSentrySubsystem::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options)
{
	TSharedPtr<SentryTransactionContextApple> transactionContextIOS = StaticCastSharedPtr<SentryTransactionContextApple>(context);

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()
		customSamplingContext:SentryConvertersApple::StringMapToNative(options)];

	return MakeShareable(new SentryTransactionApple(transaction));
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

	// `SentryId` definition was moved to Swift so its name that can be recognized by UE should be taken from "Sentry-Swift.h" to successfully load class on Mac

#if PLATFORM_MAC
	SentryId* traceId = [[SENTRY_APPLE_CLASS(_TtC6Sentry8SentryId) alloc] initWithUUIDString:traceParts[0].GetNSString()];
#elif PLATFORM_IOS
	SentryId* traceId = [[SENTRY_APPLE_CLASS(SentryId) alloc] initWithUUIDString:traceParts[0].GetNSString()];
#endif

	SentryTransactionContext* transactionContext = [[SENTRY_APPLE_CLASS(SentryTransactionContext) alloc] initWithName:@"<unlabeled transaction>" operation:@"default"
		traceId:traceId
		spanId:[[SENTRY_APPLE_CLASS(SentrySpanId) alloc] init]
		parentSpanId:[[SENTRY_APPLE_CLASS(SentrySpanId) alloc] initWithValue:traceParts[1].GetNSString()]
		parentSampled:sampleDecision];

	// currently `sentry-cocoa` doesn't have API for `SentryTransactionContext` to set `baggageHeaders`

	return MakeShareable(new SentryTransactionContextApple(transactionContext));
}
