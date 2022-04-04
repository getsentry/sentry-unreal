// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystemIOS.h"
#include "SentryScopeIOS.h"
#include "SentryEventIOS.h"
#include "Infrastructure/SentryConvertorsIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

void SentrySubsystemIOS::InitWithSettings(const USentrySettings* settings)
{
	[SentrySDK startWithConfigureOptions:^(SentryOptions *options) {
		options.dsn = settings->DsnUrl.GetNSString();
	}];
}

void SentrySubsystemIOS::AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level)
{
	SentryBreadcrumb* breadcrumb = [[SentryBreadcrumb alloc] init];

	breadcrumb.message = message.GetNSString();
	breadcrumb.level = SentryConvertorsIOS::SentryLevelToNative(level);
	breadcrumb.data = SentryConvertorsIOS::StringMapToNative(data);
	
	if(!category.IsEmpty())
		breadcrumb.category = category.GetNSString();
	if(!type.IsEmpty())
		breadcrumb.type = type.GetNSString();

	[SentrySDK addBreadcrumb:breadcrumb];
}

FString SentrySubsystemIOS::CaptureMessage(const FString& message, ESentryLevel level)
{
	SentryId* id = [SentrySDK captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
	}];

	return FString([id sentryIdString]);
}

FString SentrySubsystemIOS::CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	SentryId* id = [SentrySDK captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
		onScopeConfigure.ExecuteIfBound(SentryConvertorsIOS::SentryScopeToUnreal(scope));
	}];

	return FString([id sentryIdString]);
}

FString SentrySubsystemIOS::CaptureEvent(USentryEvent* event)
{
	TSharedPtr<SentryEventIOS> eventIOS = StaticCastSharedPtr<SentryEventIOS>(event->GetNativeImpl());

	SentryId* id = [SentrySDK captureEvent:eventIOS->GetNativeObject()];
	return FString([id sentryIdString]);
}

FString SentrySubsystemIOS::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure)
{
	TSharedPtr<SentryEventIOS> eventIOS = StaticCastSharedPtr<SentryEventIOS>(event->GetNativeImpl());

	SentryId* id = [SentrySDK captureEvent:eventIOS->GetNativeObject() withScopeBlock:^(SentryScope* scope) {
		onScopeConfigure.ExecuteIfBound(SentryConvertorsIOS::SentryScopeToUnreal(scope));
	}];

	return FString([id sentryIdString]);
}
