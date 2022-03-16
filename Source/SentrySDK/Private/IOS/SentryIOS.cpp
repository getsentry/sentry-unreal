// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIOS.h"
#include "SentryScopeIOS.h"
#include "Infrastructure/SentryConvertorsIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

void SentryIOS::InitWithSettings(const USentrySettings* settings)
{
	[SentrySDK startWithConfigureOptions:^(SentryOptions *options) {
		options.dsn = settings->DsnUrl.GetNSString();
	}];
}

void SentryIOS::AddBreadcrumb(const FString& message, const FString& category, const FString& type, const TMap<FString, FString>& data, ESentryLevel level)
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

FString SentryIOS::CaptureMessage(const FString& message, ESentryLevel level)
{
	SentryId* id = [SentrySDK captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
	}];

	return FString([id sentryIdString]);
}

FString SentryIOS::CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	SentryId* id = [SentrySDK captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];

		TSharedPtr<SentryScopeIOS> scopeNativeImpl = MakeShareable(new SentryScopeIOS());
		scopeNativeImpl->InitWithNativeObject(scope);

		onScopeConfigure.ExecuteIfBound(SentryConvertorsIOS::SentryScopeToUnreal(scopeNativeImpl));
	}];

	return FString([id sentryIdString]);
}

FString SentryIOS::CaptureEvent(USentryEvent* event)
{
	SentryId* id = [SentrySDK captureEvent:event->GetNativeImplIOS()->GetNativeObject()];
	return FString([id sentryIdString]);
}

FString SentryIOS::CaptureEventWithScope(USentryEvent* event, const FConfigureScopeDelegate& onScopeConfigure)
{
	SentryId* id = [SentrySDK captureEvent:event->GetNativeImplIOS()->GetNativeObject() withScopeBlock:^(SentryScope* scope) {
		TSharedPtr<SentryScopeIOS> scopeNativeImpl = MakeShareable(new SentryScopeIOS());
		scopeNativeImpl->InitWithNativeObject(scope);

		onScopeConfigure.ExecuteIfBound(SentryConvertorsIOS::SentryScopeToUnreal(scopeNativeImpl));
	}];

	return FString([id sentryIdString]);
}

FString SentryIOS::CaptureError()
{
	NSError* error = [NSError errorWithDomain:@"YourErrorDomain" code:0 userInfo: nil];
	SentryId* id = [SentrySDK captureError:error];
	return FString([id sentryIdString]);
}
