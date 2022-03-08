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

void SentryIOS::CaptureError()
{
	NSError* error = [NSError errorWithDomain:@"YourErrorDomain" code:0 userInfo: nil];
	[SentrySDK captureError:error];
}
