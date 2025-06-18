// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryEvent.h"
#include "AndroidSentryId.h"
#include "AndroidSentryMessage.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryEvent::FAndroidSentryEvent()
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryEvent, "()V")
{
	SetupClassMethods();
}

FAndroidSentryEvent::FAndroidSentryEvent(jobject event)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryEvent, event)
{
	SetupClassMethods();
}

void FAndroidSentryEvent::SetupClassMethods()
{
	GetIdMethod = GetMethod("getEventId", "()Lio/sentry/protocol/SentryId;");
	SetMessageMethod = GetMethod("setMessage", "(Lio/sentry/protocol/Message;)V");
	GetMessageMethod = GetMethod("getMessage", "()Lio/sentry/protocol/Message;");
	SetLevelMethod = GetMethod("setLevel", "(Lio/sentry/SentryLevel;)V");
	GetLevelMethod = GetMethod("getLevel", "()Lio/sentry/SentryLevel;");
	SetFingerprintMethod = GetMethod("setFingerprints", "(Ljava/util/List;)V");
	GetFingerprintMethod = GetMethod("getFingerprints", "()Ljava/util/List;");
	SetTagValueMethod = GetMethod("setTag", "(Ljava/lang/String;Ljava/lang/String;)V");
	GetTagValueMethod = GetMethod("getTag", "(Ljava/lang/String;)Ljava/lang/String;");
	RemoveTagMethod = GetMethod("removeTag", "(Ljava/lang/String;)V");
	SetTagsMethod = GetMethod("setTags", "(Ljava/util/Map;)V");
	GetTagsMethod = GetMethod("getTags", "()Ljava/util/Map;");
	SetExtraMethod = GetMethod("setExtra", "(Ljava/lang/String;Ljava/lang/Object;)V");
	GetExtraMethod = GetMethod("getExtra", "(Ljava/lang/String;)Ljava/lang/Object;");
	RemoveExtraMethod = GetMethod("removeExtra", "(Ljava/lang/String;)V");
	SetExtrasMethod = GetMethod("setExtras", "(Ljava/util/Map;)V");
	GetExtrasMethod = GetMethod("getExtras", "()Ljava/util/Map;");
	IsCrashMethod = GetMethod("isCrashed", "()Z");
}

TSharedPtr<ISentryId> FAndroidSentryEvent::GetId() const
{
	auto id = CallObjectMethod<jobject>(GetIdMethod);
	return MakeShareable(new FAndroidSentryId(*id));
}

void FAndroidSentryEvent::SetMessage(const FString& message)
{
	CallMethod<void>(SetMessageMethod, FAndroidSentryMessage(message).GetJObject());
}

FString FAndroidSentryEvent::GetMessage() const
{
	auto message = CallObjectMethod<jobject>(GetMessageMethod);
	return FAndroidSentryMessage(*message).ToString();
}

void FAndroidSentryEvent::SetLevel(ESentryLevel level)
{
	CallMethod<void>(SetLevelMethod, FAndroidSentryConverters::SentryLevelToNative(level)->GetJObject());
}

ESentryLevel FAndroidSentryEvent::GetLevel() const
{
	auto level = CallObjectMethod<jobject>(GetLevelMethod);
	return FAndroidSentryConverters::SentryLevelToUnreal(*level);
}

void FAndroidSentryEvent::SetFingerprint(const TArray<FString>& fingerprint)
{
	CallMethod<void>(SetFingerprintMethod, FAndroidSentryConverters::StringArrayToNative(fingerprint)->GetJObject());
}

TArray<FString> FAndroidSentryEvent::GetFingerprint()
{
	auto fingerprint = CallObjectMethod<jobject>(GetFingerprintMethod);
	return FAndroidSentryConverters::StringListToUnreal(*fingerprint);
}

void FAndroidSentryEvent::SetTag(const FString& key, const FString& value)
{
	CallMethod<void>(SetTagValueMethod, *GetJString(key), *GetJString(value));
}

FString FAndroidSentryEvent::GetTag(const FString& key) const
{
	return CallMethod<FString>(GetTagValueMethod, *GetJString(key));
}

bool FAndroidSentryEvent::TryGetTag(const FString& key, FString& value) const
{
	auto tag = CallObjectMethod<jobject>(GetTagValueMethod, *GetJString(key));

	if (!tag)
	{
		return false;
	}

	FSentryJavaObjectWrapper tagString(SentryJavaClasses::String, *tag);
	FSentryJavaMethod ToStringMethod = tagString.GetMethod("toString", "()Ljava/lang/String;");

	value = tagString.CallMethod<FString>(ToStringMethod);

	return true;
}

void FAndroidSentryEvent::RemoveTag(const FString& key)
{
	CallMethod<void>(RemoveTagMethod, *GetJString(key));
}

void FAndroidSentryEvent::SetTags(const TMap<FString, FString>& tags)
{
	CallMethod<void>(SetTagsMethod, FAndroidSentryConverters::StringMapToNative(tags)->GetJObject());
}

TMap<FString, FString> FAndroidSentryEvent::GetTags() const
{
	auto tags = CallObjectMethod<jobject>(GetTagsMethod);
	return FAndroidSentryConverters::StringMapToUnreal(*tags);
}

void FAndroidSentryEvent::SetContext(const FString& key, const TMap<FString, FSentryVariant>& values)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setContext", "(Lio/sentry/SentryEvent;Ljava/lang/String;Ljava/lang/Object;)V",
		GetJObject(), *FSentryJavaObjectWrapper::GetJString(key), FAndroidSentryConverters::VariantMapToNative(values)->GetJObject());
}

TMap<FString, FSentryVariant> FAndroidSentryEvent::GetContext(const FString& key) const
{
	auto context = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "getContext", "(Lio/sentry/SentryEvent;Ljava/lang/String;)Ljava/lang/Object;",
		GetJObject(), *FSentryJavaObjectWrapper::GetJString(key));
	return FAndroidSentryConverters::VariantMapToUnreal(*context);
}

bool FAndroidSentryEvent::TryGetContext(const FString& key, TMap<FString, FSentryVariant>& value) const
{
	auto context = FSentryJavaObjectWrapper::CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "getContext", "(Lio/sentry/SentryEvent;Ljava/lang/String;)Ljava/lang/Object;",
		GetJObject(), *FSentryJavaObjectWrapper::GetJString(key));

	if (!context)
	{
		return false;
	}

	const FSentryVariant& contextVariant = FAndroidSentryConverters::VariantToUnreal(*context);
	if (contextVariant.GetType() == ESentryVariantType::Empty)
	{
		return false;
	}

	value = contextVariant.GetValue<TMap<FString, FSentryVariant>>();

	return true;
}

void FAndroidSentryEvent::RemoveContext(const FString& key)
{
	FSentryJavaObjectWrapper::CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "removeContext", "(Lio/sentry/SentryEvent;Ljava/lang/String;)V",
		GetJObject(), *FSentryJavaObjectWrapper::GetJString(key));
}

void FAndroidSentryEvent::SetExtra(const FString& key, const FSentryVariant& value)
{
	CallMethod<void>(SetExtraMethod, *GetJString(key), FAndroidSentryConverters::VariantToNative(value)->GetJObject());
}

FSentryVariant FAndroidSentryEvent::GetExtra(const FString& key) const
{
	auto extra = CallObjectMethod<jobject>(GetExtraMethod, *GetJString(key));
	return FAndroidSentryConverters::VariantToUnreal(*extra);
}

bool FAndroidSentryEvent::TryGetExtra(const FString& key, FSentryVariant& value) const
{
	auto extra = CallObjectMethod<jobject>(GetExtraMethod, *GetJString(key));
	if (!extra)
	{
		return false;
	}

	const FSentryVariant& extraVariant = FAndroidSentryConverters::VariantToUnreal(*extra);
	if (extraVariant.GetType() == ESentryVariantType::Empty)
	{
		return false;
	}

	value = extraVariant;

	return true;
}

void FAndroidSentryEvent::RemoveExtra(const FString& key)
{
	CallMethod<void>(RemoveExtraMethod, *GetJString(key));
}

void FAndroidSentryEvent::SetExtras(const TMap<FString, FSentryVariant>& extras)
{
	CallMethod<void>(SetExtrasMethod, FAndroidSentryConverters::VariantMapToNative(extras)->GetJObject());
}

TMap<FString, FSentryVariant> FAndroidSentryEvent::GetExtras() const
{
	auto extras = CallObjectMethod<jobject>(GetExtrasMethod);
	return FAndroidSentryConverters::VariantMapToUnreal(*extras);
}

bool FAndroidSentryEvent::IsCrash() const
{
	return CallMethod<bool>(IsCrashMethod);
}

bool FAndroidSentryEvent::IsAnr() const
{
	return CallStaticMethod<bool>(SentryJavaClasses::SentryBridgeJava, "isAnrEvent", "(Lio/sentry/SentryEvent;)Z", GetJObject());
}
