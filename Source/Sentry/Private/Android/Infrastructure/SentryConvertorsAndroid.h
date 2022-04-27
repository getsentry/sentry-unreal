// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "Android/AndroidJNI.h"

class USentryScope;
class USentryId;

class SentryConvertorsAndroid
{
public:
	/** Conversions to native Java types */
	static jobject SentryLevelToNative(ESentryLevel level);
	static jobject SentryMessageToNative(FString message);
	static jstring StringToNative(FString string);
	static jobject StringArrayToNative(const TArray<FString>& stringArray);
	static jobject StringMapToNative(const TMap<FString, FString>& stringMap);

	/** Conversions from native Java types */
	static ESentryLevel SentryLevelToUnreal(jobject level);
	static FString SentryMessageToUnreal(jobject message);
	static FString StringToUnreal(jstring string);
	static USentryScope* SentryScopeToUnreal(jobject scope);
	static USentryId* SentryIdToUnreal(jobject id);
	static TMap<FString, FString> StringMapToUnreal(jobject stringMap);
	static TArray<FString> StringListToUnreal(jobject stringList);
};
