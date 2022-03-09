// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIOS.h"
#include "SentryConvertorsIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

void SentryIOS::InitWithSettings(const USentrySettings* settings)
{
	[SentrySDK startWithConfigureOptions:^(SentryOptions *options) {
		options.dsn = settings->DsnUrl.GetNSString();
	}];
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
		onScopeConfigure.ExecuteIfBound(SentryConvertorsIOS::SentryScopeToUnreal(scope));
	}];

	return FGuid([id sentryIdString]);
}

FGuid SentryIOS::CaptureError()
{
	NSError* error = [NSError errorWithDomain:@"YourErrorDomain" code:0 userInfo: nil];
	SentryId* id = [SentrySDK captureError:error];
	return FGuid([id sentryIdString]);
}
