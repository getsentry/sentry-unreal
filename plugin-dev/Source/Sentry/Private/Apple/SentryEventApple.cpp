// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryEventApple.h"

#include "Infrastructure/SentryConvertorsApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryEventApple::SentryEventApple()
{
	EventApple = [[SENTRY_APPLE_CLASS(SentryEvent) alloc] init];
}

SentryEventApple::SentryEventApple(SentryEvent* event)
{
	EventApple = event;
}

SentryEventApple::~SentryEventApple()
{
	// Put custom destructor logic here if needed
}

SentryEvent* SentryEventApple::GetNativeObject()
{
	return EventApple;
}

void SentryEventApple::SetMessage(const FString& message)
{
	SentryMessage* msg = [SENTRY_APPLE_CLASS(SentryMessage) alloc];
	msg.message = message.GetNSString();
	EventApple.message = msg;
}

FString SentryEventApple::GetMessage() const
{
	SentryMessage* msg = EventApple.message;
	return FString(msg.message);
}

void SentryEventApple::SetLevel(ESentryLevel level)
{
	EventApple.level = SentryConvertorsApple::SentryLevelToNative(level);
}

ESentryLevel SentryEventApple::GetLevel() const
{
	return SentryConvertorsApple::SentryLevelToUnreal(EventApple.level);
}

bool SentryEventApple::IsCrash() const
{
	return EventApple.error != nullptr;
}

bool SentryEventApple::IsAnr() const
{
	bool isErrorLevel = EventApple.level == kSentryLevelError;
	bool isAppHangException = false;
	bool isAppHangMechanism = false;
	bool isAppHangMessage = false;

	if (EventApple.exceptions != nil && EventApple.exceptions.count == 1)
	{
		SentryException *exception = EventApple.exceptions[0];
		isAppHangException = [exception.type isEqualToString:@"App Hanging"];
		isAppHangMechanism = exception.mechanism != nil && [exception.mechanism.type isEqualToString:@"AppHang"];
		isAppHangMessage = [exception.value hasPrefix:@"App hanging for at least"];
	}

	return isErrorLevel && isAppHangException && isAppHangMechanism && isAppHangMessage;
}
