// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

void SentryIOS::InitWithSettings(const USentrySettings* settings)
{
	[SentrySDK startWithConfigureOptions:^(SentryOptions *options) {
		options.dsn = settings->DsnUrl.GetNSString();
	}];
}

void SentryIOS::CaptureMessage(const FString& message, ESentryLevel level)
{
	SentryLevel lvl;

	switch (level)
	{
	case ESentryLevel::Debug:
		lvl = kSentryLevelDebug;
		break;
	case ESentryLevel::Info:
		lvl = kSentryLevelInfo;
		break;
	case ESentryLevel::Warning:
		lvl = kSentryLevelWarning;
		break;
	case ESentryLevel::Error:
		lvl = kSentryLevelError;
		break;
	case ESentryLevel::Fatal:
		lvl = kSentryLevelFatal;
		break;
	default:
		NSLog(@"Unknown sentry level used while capturing message!");
	}

	SentryScope* scope = [[SentryScope alloc] init];
	[scope setLevel:lvl];

	[SentrySDK captureMessage:message.GetNSString() withScope:scope];
}

void SentryIOS::CaptureError()
{
	NSError* error = [NSError errorWithDomain:@"YourErrorDomain" code:0 userInfo: nil];
	[SentrySDK captureError:error];
}
