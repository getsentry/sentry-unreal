// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

SentryEventIOS::SentryEventIOS()
{
	EventIOS = [[SentryEvent alloc] init];
}

SentryEventIOS::SentryEventIOS(SentryEvent* event)
{
	EventIOS = event;
}

SentryEvent* SentryEventIOS::GetNativeObject()
{
	return EventIOS;
}

void SentryEventIOS::SetMessage(const FString& message)
{
	SentryMessage* msg = [SentryMessage alloc];
	msg.message = message.GetNSString();
	EventIOS.message = msg;
}

void SentryEventIOS::SetLevel(ESentryLevel level)
{
	EventIOS.level = SentryConvertorsIOS::SentryLevelToNative(level);
}