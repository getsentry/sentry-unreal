// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventIOS.h"

#include "Infrastructure/SentryConvertorsIOS.h"

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

SentryEventIOS::~SentryEventIOS()
{
	// Put custom destructor logic here if needed
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

FString SentryEventIOS::GetMessage() const
{
	SentryMessage* msg = EventIOS.message;
	return FString(msg.message);
}

void SentryEventIOS::SetLevel(ESentryLevel level)
{
	EventIOS.level = SentryConvertorsIOS::SentryLevelToNative(level);
}

ESentryLevel SentryEventIOS::GetLevel() const
{
	return SentryConvertorsIOS::SentryLevelToUnreal(EventIOS.level);
}
