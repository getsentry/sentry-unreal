// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

void SentryEventIOS::Init()
{
	EventIOS = [[SentryEvent alloc] init];
}

void SentryEventIOS::InitWithNativeObject(SentryEvent* event)
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