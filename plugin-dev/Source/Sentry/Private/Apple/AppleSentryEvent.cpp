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

void FAppleSentryEvent::SetTag(const FString& key, const FString& value)
{
	NSMutableDictionary* mutableTags = [EventApple.tags mutableCopy] ?: [NSMutableDictionary dictionary];
	mutableTags[key.GetNSString()] = value.GetNSString();
	EventApple.tags = mutableTags;
}

FString FAppleSentryEvent::GetTag(const FString& key) const
{
	return FString(EventApple.tags[key.GetNSString()]);
}

void FAppleSentryEvent::RemoveTag(const FString& key)
{
	NSMutableDictionary* mutableTags = [EventApple.tags mutableCopy] ?: [NSMutableDictionary dictionary];
	[mutableTags removeObjectForKey:key.GetNSString()];
	EventApple.tags = mutableTags;
}

void FAppleSentryEvent::SetTags(const TMap<FString, FString>& tags)
{
	EventApple.tags = FAppleSentryConverters::StringMapToNative(tags);
}

TMap<FString, FString> FAppleSentryEvent::GetTags() const
{
	return FAppleSentryConverters::StringMapToUnreal(EventApple.tags);
}

void FAppleSentryEvent::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	NSMutableDictionary* mutableContext = [EventApple.context mutableCopy] ?: [NSMutableDictionary dictionary];
	mutableContext[key.GetNSString()] = FAppleSentryConverters::VariantMapToNative(values);
	EventApple.context = mutableContext;
}

TMap<FString, FSentryVariant> FAppleSentryEvent::GetContext(const FString& key) const
{
	return FAppleSentryConverters::VariantMapToUnreal(EventApple.context[key.GetNSString()]);
}

void FAppleSentryEvent::RemoveContext(const FString& key)
{
	NSMutableDictionary* mutableContext = [EventApple.context mutableCopy] ?: [NSMutableDictionary dictionary];
	[mutableContext removeObjectForKey:key.GetNSString()];
	EventApple.context = mutableContext;
}

void FAppleSentryEvent::SetExtraValue(const FString& key, const FString& value)
{
	NSMutableDictionary* mutableExtra = [EventApple.extra mutableCopy] ?: [NSMutableDictionary dictionary];
	mutableExtra[key.GetNSString()] = value.GetNSString();
	EventApple.extra = mutableExtra;
}

FString FAppleSentryEvent::GetExtraValue(const FString& key) const
{
	return FString(EventApple.extra[key.GetNSString()]);
}

void FAppleSentryEvent::RemoveExtra(const FString& key)
{
	NSMutableDictionary* mutableExtra = [EventApple.extra mutableCopy] ?: [NSMutableDictionary dictionary];
	[mutableExtra removeObjectForKey:key.GetNSString()];
	EventApple.extra = mutableExtra;
}

void FAppleSentryEvent::SetExtras(const TMap<FString, FString>& extras)
{
	EventApple.extra = FAppleSentryConverters::StringMapToNative(extras);
}

TMap<FString, FString> FAppleSentryEvent::GetExtras() const
{
	return FAppleSentryConverters::StringMapToUnreal(EventApple.extra);
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
