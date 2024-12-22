// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemApple.h"

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

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/FileManager.h"
#include "UObject/GarbageCollection.h"
#include "Utils/SentryLogUtils.h"

void SentrySubsystemApple::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler)
{
	[SENTRY_APPLE_CLASS(PrivateSentrySDKOnly) setSdkName:@"sentry.cocoa.unreal"];

	dispatch_group_t sentryDispatchGroup = dispatch_group_create();
	dispatch_group_enter(sentryDispatchGroup);
	dispatch_async(dispatch_get_main_queue(), ^{
		[SENTRY_APPLE_CLASS(SentrySDK) startWithConfigureOptions:^(SentryOptions *options) {
			options.dsn = settings->Dsn.GetNSString();
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
				FGCScopeGuard GCScopeGuard;
				USentryEvent* EventToProcess = NewObject<USentryEvent>();
				EventToProcess->InitWithNativeImpl(MakeShareable(new SentryEventApple(event)));
				return beforeSendHandler->HandleBeforeSend(EventToProcess, nullptr) ? event : nullptr;
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
					USentrySamplingContext* Context = NewObject<USentrySamplingContext>();
					Context->InitWithNativeImpl(MakeShareable(new SentrySamplingContextApple(samplingContext)));
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

void SentrySubsystemApple::Close()
{
	[SENTRY_APPLE_CLASS(SentrySDK) close];
}

bool SentrySubsystemApple::IsEnabled()
{
	return [SENTRY_APPLE_CLASS(SentrySDK) isEnabled];
}

ESentryCrashedLastRun SentrySubsystemApple::IsCrashedLastRun()
{
	return [SENTRY_APPLE_CLASS(SentrySDK) crashedLastRun] ? ESentryCrashedLastRun::Crashed : ESentryCrashedLastRun::NotCrashed;
}

void SentrySubsystemApple::AddBreadcrumb(TSharedPtr<ISentryBreadcrumb> breadcrumb)
{
	TSharedPtr<SentryBreadcrumbApple> breadcrumbIOS = StaticCastSharedPtr<SentryBreadcrumbApple>(breadcrumb);

	[SENTRY_APPLE_CLASS(SentrySDK) addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void SentrySubsystemApple::AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data, ESentryLevel Level)
{
	TSharedPtr<SentryBreadcrumbApple> breadcrumbIOS = MakeShareable(new SentryBreadcrumbApple());
	breadcrumbIOS->SetMessage(Message);
	breadcrumbIOS->SetCategory(Category);
	breadcrumbIOS->SetType(Type);
	breadcrumbIOS->SetData(Data);
	breadcrumbIOS->SetLevel(Level);

	[SENTRY_APPLE_CLASS(SentrySDK) addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void SentrySubsystemApple::ClearBreadcrumbs()
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope clearBreadcrumbs];
	}];
}

TSharedPtr<ISentryId> SentrySubsystemApple::CaptureMessage(const FString& message, ESentryLevel level)
{
	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsApple::SentryLevelToNative(level)];
	}];

	return MakeShareable(new SentryIdApple(id));
}

TSharedPtr<ISentryId> SentrySubsystemApple::CaptureMessageWithScope(const FString& message, const FSentryScopeDelegate& onConfigureScope, ESentryLevel level)
{
	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsApple::SentryLevelToNative(level)];
		onConfigureScope.ExecuteIfBound(MakeShareable(new SentryScopeApple(scope)));
	}];

	return MakeShareable(new SentryIdApple(id));
}

TSharedPtr<ISentryId> SentrySubsystemApple::CaptureEvent(TSharedPtr<ISentryEvent> event)
{
	TSharedPtr<SentryEventApple> eventIOS = StaticCastSharedPtr<SentryEventApple>(event);

	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:eventIOS->GetNativeObject()];
	return MakeShareable(new SentryIdApple(id));
}

TSharedPtr<ISentryId> SentrySubsystemApple::CaptureEventWithScope(TSharedPtr<ISentryEvent> event, const FSentryScopeDelegate& onConfigureScope)
{
	TSharedPtr<SentryEventApple> eventIOS = StaticCastSharedPtr<SentryEventApple>(event);

	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:eventIOS->GetNativeObject() withScopeBlock:^(SentryScope* scope) {
		onConfigureScope.ExecuteIfBound(MakeShareable(new SentryScopeApple(scope)));
	}];

	return MakeShareable(new SentryIdApple(id));
}

TSharedPtr<ISentryId> SentrySubsystemApple::CaptureException(const FString& type, const FString& message, int32 framesToSkip)
{
	auto StackFrames = FGenericPlatformStackWalk::GetStack(framesToSkip);

	SentryException *nativeException = [[SENTRY_APPLE_CLASS(SentryException) alloc] initWithValue:message.GetNSString() type:type.GetNSString()];
	NSMutableArray *nativeExceptionArray = [NSMutableArray arrayWithCapacity:1];
	[nativeExceptionArray addObject:nativeException];

	SentryEvent *exceptionEvent = [[SENTRY_APPLE_CLASS(SentryEvent) alloc] init];
	exceptionEvent.exceptions = nativeExceptionArray;
	exceptionEvent.stacktrace = SentryConvertorsApple::CallstackToNative(StackFrames);
	
	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:exceptionEvent];
	return MakeShareable(new SentryIdApple(id));
}

TSharedPtr<ISentryId> SentrySubsystemApple::CaptureAssertion(const FString& type, const FString& message)
{
#if PLATFORM_MAC
	int32 framesToSkip = 6;
#elif PLATFORM_IOS
	int32 framesToSkip = 5;
#endif

	SentryLogUtils::LogStackTrace(*message, ELogVerbosity::Error, framesToSkip);

	return CaptureException(type, message, framesToSkip);
}

TSharedPtr<ISentryId> SentrySubsystemApple::CaptureEnsure(const FString& type, const FString& message)
{
	int32 framesToSkip = 6;

	SentryLogUtils::LogStackTrace(*message, ELogVerbosity::Error, framesToSkip);

	return CaptureException(type, message, framesToSkip);
}

void SentrySubsystemApple::CaptureUserFeedback(TSharedPtr<ISentryUserFeedback> userFeedback)
{
	TSharedPtr<SentryUserFeedbackApple> userFeedbackIOS = StaticCastSharedPtr<SentryUserFeedbackApple>(userFeedback);

	[SENTRY_APPLE_CLASS(SentrySDK) captureUserFeedback:userFeedbackIOS->GetNativeObject()];
}

void SentrySubsystemApple::SetUser(TSharedPtr<ISentryUser> user)
{
	TSharedPtr<SentryUserApple> userIOS = StaticCastSharedPtr<SentryUserApple>(user);

	[SENTRY_APPLE_CLASS(SentrySDK) setUser:userIOS->GetNativeObject()];
}

void SentrySubsystemApple::RemoveUser()
{
	[SENTRY_APPLE_CLASS(SentrySDK) setUser:nil];
}

void SentrySubsystemApple::ConfigureScope(const FSentryScopeDelegate& onConfigureScope)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		onConfigureScope.ExecuteIfBound(MakeShareable(new SentryScopeApple(scope)));
	}];
}

void SentrySubsystemApple::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope setContextValue:SentryConvertorsApple::StringMapToNative(values) forKey:key.GetNSString()];
	}];
}

void SentrySubsystemApple::SetTag(const FString& key, const FString& value)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope setTagValue:value.GetNSString() forKey:key.GetNSString()];
	}];
}

void SentrySubsystemApple::RemoveTag(const FString& key)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope removeTagForKey:key.GetNSString()];
	}];
}

void SentrySubsystemApple::SetLevel(ESentryLevel level)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope setLevel:SentryConvertorsApple::SentryLevelToNative(level)];
	}];
}

void SentrySubsystemApple::StartSession()
{
	[SENTRY_APPLE_CLASS(SentrySDK) startSession];
}

void SentrySubsystemApple::EndSession()
{
	[SENTRY_APPLE_CLASS(SentrySDK) endSession];
}

TSharedPtr<ISentryTransaction> SentrySubsystemApple::StartTransaction(const FString& name, const FString& operation)
{
	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithName:name.GetNSString() operation:operation.GetNSString()];

	return MakeShareable(new SentryTransactionApple(transaction));
}

TSharedPtr<ISentryTransaction> SentrySubsystemApple::StartTransactionWithContext(TSharedPtr<ISentryTransactionContext> context)
{
	TSharedPtr<SentryTransactionContextApple> transactionContextIOS = StaticCastSharedPtr<SentryTransactionContextApple>(context);

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()];

	return MakeShareable(new SentryTransactionApple(transaction));
}

TSharedPtr<ISentryTransaction> SentrySubsystemApple::StartTransactionWithContextAndTimestamp(TSharedPtr<ISentryTransactionContext> context, int64 timestamp)
{
	UE_LOG(LogSentrySdk, Log, TEXT("Setting transaction timestamp explicitly not supported on Mac/iOS."));
	return StartTransactionWithContext(context);
}

TSharedPtr<ISentryTransaction> SentrySubsystemApple::StartTransactionWithContextAndOptions(TSharedPtr<ISentryTransactionContext> context, const TMap<FString, FString>& options)
{
	TSharedPtr<SentryTransactionContextApple> transactionContextIOS = StaticCastSharedPtr<SentryTransactionContextApple>(context);

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()
		customSamplingContext:SentryConvertorsApple::StringMapToNative(options)];

	return MakeShareable(new SentryTransactionApple(transaction));
}

TSharedPtr<ISentryTransactionContext> SentrySubsystemApple::ContinueTrace(const FString& sentryTrace, const TArray<FString>& baggageHeaders)
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
