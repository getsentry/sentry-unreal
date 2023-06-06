// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemApple.h"

#include "SentryBreadcrumbApple.h"
#include "SentryEventApple.h"
#include "SentryScopeApple.h"
#include "SentryUserApple.h"
#include "SentryUserFeedbackApple.h"

#include "SentryEvent.h"
#include "SentryBreadcrumb.h"
#include "SentryId.h"
#include "SentrySettings.h"
#include "SentryUserFeedback.h"
#include "SentryUser.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

#include "GenericPlatform/GenericPlatformOutputDevices.h"
#include "HAL/FileManager.h"

void SentrySubsystemApple::InitWithSettings(const USentrySettings* settings)
{
	[SENTRY_APPLE_CLASS(PrivateSentrySDKOnly) setSdkName:@"sentry.cocoa.unreal"];

	[SENTRY_APPLE_CLASS(SentrySDK) startWithConfigureOptions:^(SentryOptions *options) {
		options.dsn = settings->DsnUrl.GetNSString();
		options.environment = settings->Environment.GetNSString();
		options.enableAutoSessionTracking = settings->EnableAutoSessionTracking;
		options.sessionTrackingIntervalMillis = settings->SessionTimeout;
		options.releaseName = settings->OverrideReleaseName
			? settings->Release.GetNSString()
			: settings->GetFormattedReleaseName().GetNSString();
	}];

	[SENTRY_APPLE_CLASS(SentrySDK) configureScope:^(SentryScope* scope) {
		if(settings->EnableAutoLogAttachment) {
			const FString logFilePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FGenericPlatformOutputDevices::GetAbsoluteLogFilename());
			SentryAttachment* logAttachment = [[SENTRY_APPLE_CLASS(SentryAttachment) alloc] initWithPath:logFilePath.GetNSString()];
			[scope addAttachment:logAttachment];
		}
	}];
}

void SentrySubsystemApple::Close()
{
	[SENTRY_APPLE_CLASS(SentrySDK) close];
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
