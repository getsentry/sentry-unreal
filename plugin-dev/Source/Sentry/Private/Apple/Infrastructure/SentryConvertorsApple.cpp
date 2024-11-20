// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryConvertorsApple.h"

#include "SentryDefines.h"

#include "Apple/SentryScopeApple.h"

#include "Convenience/SentryMacro.h"

SentryLevel SentryConvertorsApple::SentryLevelToNative(ESentryLevel level)
{
	SentryLevel nativeLevel = kSentryLevelDebug;

	switch (level)
	{
	case ESentryLevel::Debug:
		nativeLevel = kSentryLevelDebug;
		break;
	case ESentryLevel::Info:
		nativeLevel = kSentryLevelInfo;
		break;
	case ESentryLevel::Warning:
		nativeLevel = kSentryLevelWarning;
		break;
	case ESentryLevel::Error:
		nativeLevel = kSentryLevelError;
		break;
	case ESentryLevel::Fatal:
		nativeLevel = kSentryLevelFatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return nativeLevel;
}

NSDictionary* SentryConvertorsApple::StringMapToNative(const TMap<FString, FString>& map)
{
	NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:map.Num()];

	for (auto it = map.CreateConstIterator(); it; ++it)
	{
		[dict setValue:it.Value().GetNSString() forKey:it.Key().GetNSString()];
	}

	return dict;
}

NSArray* SentryConvertorsApple::StringArrayToNative(const TArray<FString>& array)
{
	NSMutableArray *arr = [NSMutableArray arrayWithCapacity:array.Num()];

	for (auto it = array.CreateConstIterator(); it; ++it)
	{
		[arr addObject:it->GetNSString()];
	}

	return arr;
}

NSData* SentryConvertorsApple::ByteDataToNative(const TArray<uint8>& array)
{
	return [NSData dataWithBytes:array.GetData() length:array.Num()];
}

SentryStacktrace* SentryConvertorsApple::CallstackToNative(const TArray<FProgramCounterSymbolInfo>& callstack)
{
	int32 framesCount = callstack.Num();

	NSMutableArray *arr = [NSMutableArray arrayWithCapacity:framesCount];

	for (int i = 0; i < framesCount; ++i)
	{
		SentryFrame *frame = [[SENTRY_APPLE_CLASS(SentryFrame) alloc] init];
		frame.instructionAddress = FString::Printf(TEXT("0x%llx"), callstack[framesCount - i - 1].ProgramCounter).GetNSString();
		[arr addObject:frame];
	}

	SentryStacktrace *trace = [[SENTRY_APPLE_CLASS(SentryStacktrace) alloc] initWithFrames:arr registers:@{}];

	return trace;
}

ESentryLevel SentryConvertorsApple::SentryLevelToUnreal(SentryLevel level)
{
	ESentryLevel unrealLevel = ESentryLevel::Debug;

	switch (level)
	{
	case kSentryLevelDebug:
		unrealLevel = ESentryLevel::Debug;
		break;
	case kSentryLevelInfo:
		unrealLevel = ESentryLevel::Info;
		break;
	case kSentryLevelWarning:
		unrealLevel = ESentryLevel::Warning;
		break;
	case kSentryLevelError:
		unrealLevel = ESentryLevel::Error;
		break;
	case kSentryLevelFatal:
		unrealLevel = ESentryLevel::Fatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return unrealLevel;
}

TMap<FString, FString> SentryConvertorsApple::StringMapToUnreal(NSDictionary* dict)
{
	TMap<FString, FString> map;

	for(id key in dict)
	{
		map.Add(FString(key), FString(dict[key]));
	}

	return map;
}

TArray<FString> SentryConvertorsApple::StringArrayToUnreal(NSArray* array)
{
	TArray<FString> arr;

	for (id object in array)
	{
		arr.Add(FString(object));
	}

	return arr;
}

TArray<uint8> SentryConvertorsApple::ByteDataToUnreal(NSData* data)
{
	TArray<uint8> ByteData;

	uint8* ByteArray = (uint8*)data.bytes;

	for (int i = 0; i < data.length; i++)
	{
		ByteData.Add(ByteArray[i]);
	}

	return ByteData;
}

SentryLevel SentryConvertorsApple::StringToSentryLevel(NSString* string)
{
	SentryLevel nativeLevel = kSentryLevelDebug;

	if ([string isEqualToString:@"debug"]) {
		nativeLevel = kSentryLevelDebug;
	}
	else if ([string isEqualToString:@"info"]) {
		nativeLevel = kSentryLevelInfo;
	}
	else if ([string isEqualToString:@"warning"]) {
		nativeLevel = kSentryLevelWarning;
	}
	else if ([string isEqualToString:@"error"]) {
		nativeLevel = kSentryLevelError;
	}
	else if ([string isEqualToString:@"fatal"]) {
		nativeLevel = kSentryLevelFatal;
	}
	else {
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return nativeLevel;
}