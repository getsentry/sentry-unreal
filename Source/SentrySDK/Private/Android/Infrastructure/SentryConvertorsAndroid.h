// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJNI.h"

class SentryScopeAndroid;
class USentryScope;

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
	static FString StringToUnreal(jstring string);
	static USentryScope* SentryScopeToUnreal(TSharedPtr<SentryScopeAndroid> scope);
};
