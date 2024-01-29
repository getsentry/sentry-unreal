// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "Android/AndroidJNI.h"

class USentryScope;
class USentryId;
class USentryTransaction;
class USentrySpan;
class USentryTransactionContext;
class FSentryJavaObjectWrapper;
class FJsonValue;

class SentryConvertorsAndroid
{
public:
	/** Conversions to native Java types */
	static TSharedPtr<FSentryJavaObjectWrapper> SentryLevelToNative(ESentryLevel level);
	static TSharedPtr<FSentryJavaObjectWrapper> StringArrayToNative(const TArray<FString>& stringArray);
	static TSharedPtr<FSentryJavaObjectWrapper> StringMapToNative(const TMap<FString, FString>& stringMap);
	static jbyteArray ByteArrayToNative(const TArray<uint8>& byteArray);

	/** Conversions from native Java types */
	static ESentryLevel SentryLevelToUnreal(jobject level);
	static USentryScope* SentryScopeToUnreal(jobject scope);
	static USentryId* SentryIdToUnreal(jobject id);
	static USentryTransaction* SentryTransactionToUnreal(jobject transaction);
	static USentrySpan* SentrySpanToUnreal(jobject span);
	static USentryTransactionContext* SentryTransactionContextToUnreal(jobject transactionContext);
	static TMap<FString, FString> StringMapToUnreal(jobject stringMap);
	static TArray<FString> StringListToUnreal(jobject stringList);
	static TArray<uint8> ByteArrayToUnreal(jbyteArray byteArray);

	/** Other conversions */
	static TArray<TSharedPtr<FJsonValue>> StrinArrayToJsonArray(const TArray<FString>& stringArray);
};
