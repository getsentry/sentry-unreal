// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryEvent.h"
#include "AppleSentryId.h"

#include "Infrastructure/AppleSentryConverters.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryEvent::FAppleSentryEvent()
{
	EventApple = [[SENTRY_APPLE_CLASS(SentryEvent) alloc] init];
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
	SentryMessage* msg = [SENTRY_APPLE_CLASS(SentryMessage) alloc];
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

bool FAppleSentryEvent::TryGetTag(const FString& key, FString& value) const
{
	NSString* tag = [EventApple.tags objectForKey:key.GetNSString()];

	if (!tag)
	{
		return false;
	}

	value = FString(tag);
	return true;
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

bool FAppleSentryEvent::TryGetContext(const FString& key, TMap<FString, FSentryVariant>& value) const
{
	NSDictionary* context = [EventApple.context objectForKey:key.GetNSString()];

	if (!context)
	{
		return false;
	}

	const FSentryVariant& contextVariant = FAppleSentryConverters::VariantToUnreal(context);
	if (contextVariant.GetType() == ESentryVariantType::Empty)
	{
		return false;
	}

	value = contextVariant.GetValue<TMap<FString, FSentryVariant>>();

	return true;
}

void FAppleSentryEvent::RemoveContext(const FString& key)
{
	NSMutableDictionary* mutableContext = [EventApple.context mutableCopy] ?: [NSMutableDictionary dictionary];
	[mutableContext removeObjectForKey:key.GetNSString()];
	EventApple.context = mutableContext;
}

void FAppleSentryEvent::SetExtra(const FString& key, const FSentryVariant& value)
{
	NSMutableDictionary* mutableExtra = [EventApple.extra mutableCopy] ?: [NSMutableDictionary dictionary];
	mutableExtra[key.GetNSString()] = FAppleSentryConverters::VariantToNative(value);
	EventApple.extra = mutableExtra;
}

FSentryVariant FAppleSentryEvent::GetExtra(const FString& key) const
{
	return FAppleSentryConverters::VariantToUnreal(EventApple.extra[key.GetNSString()]);
}

bool FAppleSentryEvent::TryGetExtra(const FString& key, FSentryVariant& value) const
{
	id extra = [EventApple.extra objectForKey:key.GetNSString()];

	if (!extra)
	{
		return false;
	}

	value = FAppleSentryConverters::VariantToUnreal(extra);
	return true;
}

void FAppleSentryEvent::RemoveExtra(const FString& key)
{
	NSMutableDictionary* mutableExtra = [EventApple.extra mutableCopy] ?: [NSMutableDictionary dictionary];
	[mutableExtra removeObjectForKey:key.GetNSString()];
	EventApple.extra = mutableExtra;
}

void FAppleSentryEvent::SetExtras(const TMap<FString, FSentryVariant>& extras)
{
	EventApple.extra = FAppleSentryConverters::VariantMapToNative(extras);
}

TMap<FString, FSentryVariant> FAppleSentryEvent::GetExtras() const
{
	return FAppleSentryConverters::VariantMapToUnreal(EventApple.extra);
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
