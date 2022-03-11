// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

void SentryEventIOS::Init()
{
	_eventIOS = [[SentryEvent alloc] init];
}

void SentryEventIOS::InitWithNativeObject(SentryEvent* event)
{
	_eventIOS = event;
}

SentryEvent* SentryEventIOS::GetNativeObject()
{
	return _eventIOS;
}

void SentryEventIOS::SetMessage(const FString& message)
{
	SentryMessage* msg = [SentryMessage alloc];
	msg.message = message.GetNSString();
	_eventIOS.message = msg;
}

void SentryEventIOS::SetLevel(ESentryLevel level)
{
	_eventIOS.level = SentryConvertorsIOS::SentryLevelToNative(level);
}