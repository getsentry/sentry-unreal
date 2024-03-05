// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemApple.h"

#include "SentryBreadcrumbApple.h"
#include "SentryEventApple.h"
#include "SentryScopeApple.h"
#include "SentryUserApple.h"
#include "SentryUserFeedbackApple.h"
#include "SentryTransactionApple.h"
#include "SentrySamplingContextApple.h"
#include "SentryTransactionContextApple.h"

#include "SentryEvent.h"
#include "SentryBreadcrumb.h"
#include "SentryId.h"
#include "SentrySettings.h"
#include "SentryUserFeedback.h"
#include "SentryUser.h"
#include "SentryTransaction.h"
#include "SentryBeforeSendHandler.h"
#include "SentryDefines.h"
#include "SentrySamplingContext.h"
#include "SentryTraceSampler.h"
#include "SentryTransactionContext.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/FileManager.h"

void SentrySubsystemApple::InitWithSettings(const USentrySettings* settings, USentryBeforeSendHandler* beforeSendHandler, USentryTraceSampler* traceSampler)
{
	[SENTRY_APPLE_CLASS(PrivateSentrySDKOnly) setSdkName:@"sentry.cocoa.unreal"];

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
#if PLATFORM_IOS
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
		options.enableTracing = settings->EnableTracing;
		if(settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::UniformSampleRate)
		{
			options.tracesSampleRate = [NSNumber numberWithFloat:settings->TracesSampleRate];
		}
		if(settings->EnableTracing && settings->SamplingType == ESentryTracesSamplingType::TracesSampler)
		{
			options.tracesSampler = ^NSNumber* (SentrySamplingContext* samplingContext) {
				USentrySamplingContext* Context = NewObject<USentrySamplingContext>();
				Context->InitWithNativeImpl(MakeShareable(new SentrySamplingContextApple(samplingContext)));
				float samplingValue;
				return traceSampler->Sample(Context, samplingValue) ? [NSNumber numberWithFloat:samplingValue] : nil;
			};
		}
	}];
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

void SentrySubsystemApple::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	TSharedPtr<SentryBreadcrumbApple> breadcrumbIOS = StaticCastSharedPtr<SentryBreadcrumbApple>(breadcrumb->GetNativeImpl());

	[SENTRY_APPLE_CLASS(SentrySDK) addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void SentrySubsystemApple::ClearBreadcrumbs()
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		[scope clearBreadcrumbs];
	}];
}

USentryId* SentrySubsystemApple::CaptureMessage(const FString& message, ESentryLevel level)
{
	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsApple::SentryLevelToNative(level)];
	}];

	return SentryConvertorsApple::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemApple::CaptureMessageWithScope(const FString& message, const FConfigureScopeDelegate& onConfigureScope, ESentryLevel level)
{
	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsApple::SentryLevelToNative(level)];
		onConfigureScope.ExecuteIfBound(SentryConvertorsApple::SentryScopeToUnreal(scope));
	}];

	return SentryConvertorsApple::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemApple::CaptureEvent(USentryEvent* event)
{
	TSharedPtr<SentryEventApple> eventIOS = StaticCastSharedPtr<SentryEventApple>(event->GetNativeImpl());

	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:eventIOS->GetNativeObject()];
	return SentryConvertorsApple::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemApple::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onConfigureScope)
{
	TSharedPtr<SentryEventApple> eventIOS = StaticCastSharedPtr<SentryEventApple>(event->GetNativeImpl());

	SentryId* id = [SENTRY_APPLE_CLASS(SentrySDK) captureEvent:eventIOS->GetNativeObject() withScopeBlock:^(SentryScope* scope) {
		onConfigureScope.ExecuteIfBound(SentryConvertorsApple::SentryScopeToUnreal(scope));
	}];

	return SentryConvertorsApple::SentryIdToUnreal(id);
}

void SentrySubsystemApple::CaptureUserFeedback(USentryUserFeedback* userFeedback)
{
	TSharedPtr<SentryUserFeedbackApple> userFeedbackIOS = StaticCastSharedPtr<SentryUserFeedbackApple>(userFeedback->GetNativeImpl());

	[SENTRY_APPLE_CLASS(SentrySDK) captureUserFeedback:userFeedbackIOS->GetNativeObject()];
}

void SentrySubsystemApple::SetUser(USentryUser* user)
{
	TSharedPtr<SentryUserApple> userIOS = StaticCastSharedPtr<SentryUserApple>(user->GetNativeImpl());

	[SENTRY_APPLE_CLASS(SentrySDK) setUser:userIOS->GetNativeObject()];
}

void SentrySubsystemApple::RemoveUser()
{
	[SENTRY_APPLE_CLASS(SentrySDK) setUser:nil];
}

void SentrySubsystemApple::ConfigureScope(const FConfigureScopeDelegate& onConfigureScope)
{
	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		onConfigureScope.ExecuteIfBound(SentryConvertorsApple::SentryScopeToUnreal(scope));
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

USentryTransaction* SentrySubsystemApple::StartTransaction(const FString& name, const FString& operation)
{
	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithName:name.GetNSString() operation:operation.GetNSString()];

	return SentryConvertorsApple::SentryTransactionToUnreal(transaction);
}

USentryTransaction* SentrySubsystemApple::StartTransactionWithContext(USentryTransactionContext* context)
{
	TSharedPtr<SentryTransactionContextApple> transactionContextIOS = StaticCastSharedPtr<SentryTransactionContextApple>(context->GetNativeImpl());

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()];

	return SentryConvertorsApple::SentryTransactionToUnreal(transaction);
}

USentryTransaction* SentrySubsystemApple::StartTransactionWithContextAndOptions(USentryTransactionContext* context, const TMap<FString, FString>& options)
{
	TSharedPtr<SentryTransactionContextApple> transactionContextIOS = StaticCastSharedPtr<SentryTransactionContextApple>(context->GetNativeImpl());

	id<SentrySpan> transaction = [SENTRY_APPLE_CLASS(SentrySDK) startTransactionWithContext:transactionContextIOS->GetNativeObject()
		customSamplingContext:SentryConvertorsApple::StringMapToNative(options)];

	return SentryConvertorsApple::SentryTransactionToUnreal(transaction);
}
