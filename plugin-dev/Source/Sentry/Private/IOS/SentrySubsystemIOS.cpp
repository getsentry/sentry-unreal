// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemIOS.h"
#include "SentryBreadcrumbIOS.h"
#include "SentryEventIOS.h"
#include "SentryScopeIOS.h"
#include "SentryUserIOS.h"
#include "SentryUserFeedbackIOS.h"

#include "SentryEvent.h"
#include "SentryBreadcrumb.h"
#include "SentryId.h"
#include "SentrySettings.h"
#include "SentryUserFeedback.h"
#include "SentryUser.h"

#include "Infrastructure/SentryConvertorsIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>
#import <Sentry/PrivateSentrySDKOnly.h>

void SentrySubsystemIOS::InitWithSettings(const USentrySettings* settings)
{
	[PrivateSentrySDKOnly setSdkName:@"sentry.cocoa.unreal"];

	[SentrySDK startWithConfigureOptions:^(SentryOptions *options) {
		options.dsn = settings->DsnUrl.GetNSString();
		options.releaseName = settings->Release.GetNSString();
	}];
}

void SentrySubsystemIOS::Close()
{
	[SentrySDK close];
}

void SentrySubsystemIOS::AddBreadcrumb(USentryBreadcrumb* breadcrumb)
{
	TSharedPtr<SentryBreadcrumbIOS> breadcrumbIOS = StaticCastSharedPtr<SentryBreadcrumbIOS>(breadcrumb->GetNativeImpl());

	[SentrySDK addBreadcrumb:breadcrumbIOS->GetNativeObject()];
}

void SentrySubsystemIOS::ClearBreadcrumbs()
{
	[SentrySDK configureScope:^(SentryScope* scope) {
		[scope clearBreadcrumbs];
	}];
}

USentryId* SentrySubsystemIOS::CaptureMessage(const FString& message, ESentryLevel level)
{
	SentryId* id = [SentrySDK captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
	}];

	return SentryConvertorsIOS::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemIOS::CaptureMessageWithScope(const FString& message, const FConfigureScopeDelegate& onConfigureScope, ESentryLevel level)
{
	SentryId* id = [SentrySDK captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
		onConfigureScope.ExecuteIfBound(SentryConvertorsIOS::SentryScopeToUnreal(scope));
	}];

	return SentryConvertorsIOS::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemIOS::CaptureEvent(USentryEvent* event)
{
	TSharedPtr<SentryEventIOS> eventIOS = StaticCastSharedPtr<SentryEventIOS>(event->GetNativeImpl());

	SentryId* id = [SentrySDK captureEvent:eventIOS->GetNativeObject()];
	return SentryConvertorsIOS::SentryIdToUnreal(id);
}

USentryId* SentrySubsystemIOS::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onConfigureScope)
{
	TSharedPtr<SentryEventIOS> eventIOS = StaticCastSharedPtr<SentryEventIOS>(event->GetNativeImpl());

	SentryId* id = [SentrySDK captureEvent:eventIOS->GetNativeObject() withScopeBlock:^(SentryScope* scope) {
		onConfigureScope.ExecuteIfBound(SentryConvertorsIOS::SentryScopeToUnreal(scope));
	}];

	return SentryConvertorsIOS::SentryIdToUnreal(id);
}

void SentrySubsystemIOS::CaptureUserFeedback(USentryUserFeedback* userFeedback)
{
	TSharedPtr<SentryUserFeedbackIOS> userFeedbackIOS = StaticCastSharedPtr<SentryUserFeedbackIOS>(userFeedback->GetNativeImpl());

	[SentrySDK captureUserFeedback:userFeedbackIOS->GetNativeObject()];
}

void SentrySubsystemIOS::SetUser(USentryUser* user)
{
	TSharedPtr<SentryUserIOS> userIOS = StaticCastSharedPtr<SentryUserIOS>(user->GetNativeImpl());

	[SentrySDK setUser:userIOS->GetNativeObject()];
}

void SentrySubsystemIOS::RemoveUser()
{
	[SentrySDK setUser:nil];
}

void SentrySubsystemIOS::ConfigureScope(const FConfigureScopeDelegate& onConfigureScope)
{
	[SentrySDK configureScope:^(SentryScope* scope) {
		onConfigureScope.ExecuteIfBound(SentryConvertorsIOS::SentryScopeToUnreal(scope));
	}];
}

void SentrySubsystemIOS::SetContext(const FString& key, const TMap<FString, FString>& values)
{
	[SentrySDK configureScope:^(SentryScope* scope) {
		[scope setContextValue:SentryConvertorsIOS::StringMapToNative(values) forKey:key.GetNSString()];
	}];
}

void SentrySubsystemIOS::SetTag(const FString& key, const FString& value)
{
	[SentrySDK configureScope:^(SentryScope* scope) {
		[scope setTagValue:value.GetNSString() forKey:key.GetNSString()];
	}];
}

void SentrySubsystemIOS::RemoveTag(const FString& key)
{
	[SentrySDK configureScope:^(SentryScope* scope) {
		[scope removeTagForKey:key.GetNSString()];
	}];
}

void SentrySubsystemIOS::SetLevel(ESentryLevel level)
{
	[SentrySDK configureScope:^(SentryScope* scope) {
		[scope setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
	}];
}
