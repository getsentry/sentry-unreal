// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryEvent.h"
#include "AppleSentryId.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"

FAppleSentryEvent::FAppleSentryEvent()
{
	EventApple = [[SentryEvent alloc] init];
}

FAppleSentryEvent::FAppleSentryEvent(SentryEvent* event)
{
	EventApple = event;
}

FAppleSentryEvent::~FAppleSentryEvent()
{
	// Put custom destructor logic here if needed
}

SentryEvent* FAppleSentryEvent::GetNativeObject()
{
	return EventApple;
}

TSharedPtr<ISentryId> FAppleSentryEvent::GetId() const
{
	SentryId* id = EventApple.eventId;
	return MakeShareable(new FAppleSentryId(id));
}

void FAppleSentryEvent::SetMessage(const FString& message)
{
	SentryMessage* msg = [SentryMessage alloc];
	msg.message = message.GetNSString();
	EventApple.message = msg;
}

FString FAppleSentryEvent::GetMessage() const
{
	SentryMessage* msg = EventApple.message;
	return FString(msg.message);
}

void FAppleSentryEvent::SetLevel(ESentryLevel level)
{
	EventApple.level = FAppleSentryConverters::SentryLevelToNative(level);
}

ESentryLevel FAppleSentryEvent::GetLevel() const
{
	return FAppleSentryConverters::SentryLevelToUnreal(EventApple.level);
}

void FAppleSentryEvent::SetFingerprint(const TArray<FString>& fingerprint)
{
	EventApple.fingerprint = FAppleSentryConverters::StringArrayToNative(fingerprint);
}

TArray<FString> FAppleSentryEvent::GetFingerprint()
{
	return FAppleSentryConverters::StringArrayToUnreal(EventApple.fingerprint);
}

bool FAppleSentryEvent::IsCrash() const
{
	return EventApple.error != nullptr;
}

bool FAppleSentryEvent::IsAnr() const
{
	bool isErrorLevel = EventApple.level == kSentryLevelError;
	bool isAppHangException = false;
	bool isAppHangMechanism = false;
	bool isAppHangMessage = false;

	if (EventApple.exceptions != nil && EventApple.exceptions.count == 1)
	{
		SentryException* exception = EventApple.exceptions[0];
		isAppHangException = [exception.type isEqualToString:@"App Hanging"];
		isAppHangMechanism = exception.mechanism != nil && [exception.mechanism.type isEqualToString:@"AppHang"];
		isAppHangMessage = [exception.value hasPrefix:@"App hanging for at least"];
	}

	return isErrorLevel && isAppHangException && isAppHangMechanism && isAppHangMessage;
}
