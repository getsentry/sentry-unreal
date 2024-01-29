// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "Convenience/SentryInclude.h"

class USentryTransactionContext;
class USentryScope;
class USentryId;
class USentryTransaction;
class USentrySpan;

class SentryConvertorsApple
{
public:
	/** Conversions to native iOS types */
	static SentryLevel SentryLevelToNative(ESentryLevel level);
	static NSDictionary* StringMapToNative(const TMap<FString, FString>& map);
	static NSArray* StringArrayToNative(const TArray<FString>& array);
	static NSData* ByteDataToNative(const TArray<uint8>& array);

	/** Conversions from native iOS types */
	static ESentryLevel SentryLevelToUnreal(SentryLevel level);
	static TMap<FString, FString> StringMapToUnreal(NSDictionary* dict);
	static TArray<FString> StringArrayToUnreal(NSArray* array);
	static TArray<uint8> ByteDataToUnreal(NSData* data);
	static USentryScope* SentryScopeToUnreal(SentryScope* scope);
	static USentryId* SentryIdToUnreal(SentryId* id);
	static USentryTransaction* SentryTransactionToUnreal(id<SentrySpan> transaction);
	static USentrySpan* SentrySpanToUnreal(id<SentrySpan> span);
	static USentryTransactionContext* SentryTransactionContextToUnreal(SentryTransactionContext* transactionContext);

	/** Other conversions */
	static SentryLevel StringToSentryLevel(NSString* string);
};
