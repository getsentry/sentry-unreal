// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "Android/AndroidJNI.h"

class USentryScope;
class USentryId;
class FSentryJavaObjectWrapper;

class SentryConvertorsAndroid
{
public:
	/** Conversions to native Java types */
	static TSharedPtr<FSentryJavaObjectWrapper> SentryLevelToNative(ESentryLevel level);
	static TSharedPtr<FSentryJavaObjectWrapper> SentryMessageToNative(const FString& message);
	static TSharedPtr<FSentryJavaObjectWrapper> StringArrayToNative(const TArray<FString>& stringArray);
	static TSharedPtr<FSentryJavaObjectWrapper> StringMapToNative(const TMap<FString, FString>& stringMap);
	static jbyteArray ByteArrayToNative(const TArray<uint8>& byteArray);

	/** Conversions from native Java types */
	static ESentryLevel SentryLevelToUnreal(jobject level);
	static FString SentryMessageToUnreal(jobject message);
	static USentryScope* SentryScopeToUnreal(jobject scope);
	static USentryId* SentryIdToUnreal(jobject id);
	static TMap<FString, FString> StringMapToUnreal(jobject stringMap);
	static TArray<FString> StringListToUnreal(jobject stringList);
	static TArray<uint8> ByteArrayToUnreal(jbyteArray byteArray);
};
