// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryConverters.h"

#include "SentryDefines.h"

#include "Apple/AppleSentryScope.h"
#include "Apple/Convenience/AppleSentryMacro.h"

SentryLevel FAppleSentryConverters::SentryLevelToNative(ESentryLevel level)
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

NSDictionary* FAppleSentryConverters::StringMapToNative(const TMap<FString, FString>& map)
{
	NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:map.Num()];

	for (auto it = map.CreateConstIterator(); it; ++it)
	{
		[dict setValue:it.Value().GetNSString() forKey:it.Key().GetNSString()];
	}

	return dict;
}

NSArray* FAppleSentryConverters::StringArrayToNative(const TArray<FString>& array)
{
	NSMutableArray* arr = [NSMutableArray arrayWithCapacity:array.Num()];

	for (auto it = array.CreateConstIterator(); it; ++it)
	{
		[arr addObject:it->GetNSString()];
	}

	return arr;
}

NSData* FAppleSentryConverters::ByteDataToNative(const TArray<uint8>& array)
{
	return [NSData dataWithBytes:array.GetData() length:array.Num()];
}

id FAppleSentryConverters::VariantToNative(const FSentryVariant& variant)
{
	switch (variant.GetType())
	{
	case ESentryVariantType::Integer:
		return [NSNumber numberWithInt:variant.GetValue<int32>()];
	case ESentryVariantType::Float:
		return [NSNumber numberWithFloat:variant.GetValue<float>()];
	case ESentryVariantType::Bool:
		return [NSNumber numberWithBool:variant.GetValue<bool>()];
	case ESentryVariantType::String:
		return variant.GetValue<FString>().GetNSString();
	case ESentryVariantType::Array:
		return VariantArrayToNative(variant.GetValue<TArray<FSentryVariant>>());
	case ESentryVariantType::Map:
		return VariantMapToNative(variant.GetValue<TMap<FString, FSentryVariant>>());
	default:
		return nullptr;
	}
}

NSArray* FAppleSentryConverters::VariantArrayToNative(const TArray<FSentryVariant>& variantArray)
{
	NSMutableArray* arr = [NSMutableArray arrayWithCapacity:variantArray.Num()];

	for (auto it = variantArray.CreateConstIterator(); it; ++it)
	{
		[arr addObject:VariantToNative(*it)];
	}

	return arr;
}

NSDictionary* FAppleSentryConverters::VariantMapToNative(const TMap<FString, FSentryVariant>& variantMap)
{
	NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:variantMap.Num()];

	for (auto it = variantMap.CreateConstIterator(); it; ++it)
	{
		[dict setValue:VariantToNative(it.Value()) forKey:it.Key().GetNSString()];
	}

	return dict;
}

SentryStacktrace* FAppleSentryConverters::CallstackToNative(const TArray<FProgramCounterSymbolInfo>& callstack)
{
	int32 framesCount = callstack.Num();

	NSMutableArray* arr = [NSMutableArray arrayWithCapacity:framesCount];

	for (int i = 0; i < framesCount; ++i)
	{
		SentryFrame* frame = [[SENTRY_APPLE_CLASS(SentryFrame) alloc] init];
		frame.instructionAddress = FString::Printf(TEXT("0x%llx"), callstack[framesCount - i - 1].ProgramCounter).GetNSString();
		[arr addObject:frame];
	}

	SentryStacktrace* trace = [[SENTRY_APPLE_CLASS(SentryStacktrace) alloc] initWithFrames:arr registers:@{}];

	return trace;
}

ESentryLevel FAppleSentryConverters::SentryLevelToUnreal(SentryLevel level)
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

TMap<FString, FString> FAppleSentryConverters::StringMapToUnreal(NSDictionary* dict)
{
	TMap<FString, FString> map;

	for (id key in dict)
	{
		map.Add(FString(key), FString(dict[key]));
	}

	return map;
}

TArray<FString> FAppleSentryConverters::StringArrayToUnreal(NSArray* array)
{
	TArray<FString> arr;

	for (id object in array)
	{
		arr.Add(FString(object));
	}

	return arr;
}

TArray<uint8> FAppleSentryConverters::ByteDataToUnreal(NSData* data)
{
	TArray<uint8> ByteData;

	uint8* ByteArray = (uint8*)data.bytes;

	for (int i = 0; i < data.length; i++)
	{
		ByteData.Add(ByteArray[i]);
	}

	return ByteData;
}

FSentryVariant FAppleSentryConverters::VariantToUnreal(id variant)
{
	if ([variant isKindOfClass:[NSNumber class]])
	{
		NSNumber* number = (NSNumber*)variant;

		const char* objCType = [number objCType];

		// NSNumber created from bool can be encoded as char
		if (strcmp(objCType, @encode(bool)) == 0 || strcmp(objCType, @encode(char)) == 0)
		{
			return FSentryVariant([number boolValue]);
		}
		if (strcmp(objCType, @encode(float)) == 0)
		{
			return FSentryVariant([number floatValue]);
		}
		if (strcmp(objCType, @encode(int)) == 0)
		{
			return FSentryVariant([number intValue]);
		}
	}
	if ([variant isKindOfClass:[NSString class]])
	{
		return FSentryVariant(FString(variant));
	}
	if ([variant isKindOfClass:[NSArray class]])
	{
		return VariantArrayToUnreal((NSArray*)variant);
	}
	if ([variant isKindOfClass:[NSDictionary class]])
	{
		return VariantMapToUnreal((NSDictionary*)variant);
	}

	UE_LOG(LogSentrySdk, Warning, TEXT("Couldn't convert Objective-C object to variant. Empty one will be returned."));
	return FSentryVariant();
}

TArray<FSentryVariant> FAppleSentryConverters::VariantArrayToUnreal(NSArray* variantArray)
{
	TArray<FSentryVariant> arr;

	for (id object in variantArray)
	{
		arr.Add(VariantToUnreal(object));
	}

	return arr;
}

TMap<FString, FSentryVariant> FAppleSentryConverters::VariantMapToUnreal(NSDictionary* variantMap)
{
	TMap<FString, FSentryVariant> map;

	for (id key in variantMap)
	{
		map.Add(FString(key), VariantToUnreal(variantMap[key]));
	}

	return map;
}

SentryLevel FAppleSentryConverters::StringToSentryLevel(NSString* string)
{
	SentryLevel nativeLevel = kSentryLevelDebug;

	if ([string isEqualToString:@"debug"])
	{
		nativeLevel = kSentryLevelDebug;
	}
	else if ([string isEqualToString:@"info"])
	{
		nativeLevel = kSentryLevelInfo;
	}
	else if ([string isEqualToString:@"warning"])
	{
		nativeLevel = kSentryLevelWarning;
	}
	else if ([string isEqualToString:@"error"])
	{
		nativeLevel = kSentryLevelError;
	}
	else if ([string isEqualToString:@"fatal"])
	{
		nativeLevel = kSentryLevelFatal;
	}
	else
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return nativeLevel;
}