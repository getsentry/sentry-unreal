// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIOS.h"
#include "SentryScopeIOS.h"
#include "SentryConvertorsIOS.h"

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

FGuid SentryIOS::CaptureMessage(const FString& message, ESentryLevel level)
{
	SentryId* id = [SentrySDK captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
	}];

	return FGuid([id sentryIdString]);
}

FGuid SentryIOS::CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level)
{
	SentryId* id = [SentrySDK captureMessage:message.GetNSString() withScopeBlock:^(SentryScope* scope){
		[scope setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];

		TSharedPtr<SentryScopeIOS> scopeNativeImpl = MakeShareable(new SentryScopeIOS());
		scopeNativeImpl->InitWithNativeObject(scope);

		onScopeConfigure.ExecuteIfBound(SentryConvertorsIOS::SentryScopeToUnreal(scopeNativeImpl));
	}];

	return FGuid([id sentryIdString]);
}

FGuid SentryIOS::CaptureError()
{
	NSError* error = [NSError errorWithDomain:@"YourErrorDomain" code:0 userInfo: nil];
	SentryId* id = [SentrySDK captureError:error];
	return FGuid([id sentryIdString]);
}
