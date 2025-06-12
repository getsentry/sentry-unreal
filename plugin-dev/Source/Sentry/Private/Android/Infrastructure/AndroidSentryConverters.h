// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryVariant.h"

#include "Android/AndroidJNI.h"

class FSentryJavaObjectWrapper;
class FJsonValue;

class FAndroidSentryConverters
{
public:
	/** Conversions to native Java types */
	static TSharedPtr<FSentryJavaObjectWrapper> SentryLevelToNative(ESentryLevel level);
	static TSharedPtr<FSentryJavaObjectWrapper> StringArrayToNative(const TArray<FString>& stringArray);
	static TSharedPtr<FSentryJavaObjectWrapper> StringMapToNative(const TMap<FString, FString>& stringMap);
	static TSharedPtr<FSentryJavaObjectWrapper> VariantToNative(const FSentryVariant& variant);
	static TSharedPtr<FSentryJavaObjectWrapper> VariantArrayToNative(const TArray<FSentryVariant>& variantArray);
	static TSharedPtr<FSentryJavaObjectWrapper> VariantMapToNative(const TMap<FString, FSentryVariant>& variantMap);
	static jbyteArray ByteArrayToNative(const TArray<uint8>& byteArray);

	/** Conversions from native Java types */
	static ESentryLevel SentryLevelToUnreal(jobject level);
	static TMap<FString, FString> StringMapToUnreal(jobject stringMap);
	static TArray<FString> StringListToUnreal(jobject stringList);
	static TArray<uint8> ByteArrayToUnreal(jbyteArray byteArray);
	static FSentryVariant VariantToUnreal(jobject variant);
	static TArray<FSentryVariant> VariantArrayToUnreal(jobject variantArray);
	static TMap<FString, FSentryVariant> VariantMapToUnreal(jobject variantMap);

	/** Other conversions */
	static TArray<TSharedPtr<FJsonValue>> StrinArrayToJsonArray(const TArray<FString>& stringArray);
};
