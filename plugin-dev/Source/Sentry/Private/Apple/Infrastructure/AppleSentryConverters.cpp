// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryConverters.h"

#if !USE_SENTRY_NATIVE

#include "SentryDefines.h"

#include "Apple/AppleSentryScope.h"
#include "Apple/Convenience/AppleSentryMacro.h"

SentryObjCLevel FAppleSentryConverters::SentryLevelToNative(ESentryLevel level)
{
	SentryObjCLevel nativeLevel = SentryObjCLevelDebug;

	switch (level)
	{
	case ESentryLevel::Debug:
		nativeLevel = SentryObjCLevelDebug;
		break;
	case ESentryLevel::Info:
		nativeLevel = SentryObjCLevelInfo;
		break;
	case ESentryLevel::Warning:
		nativeLevel = SentryObjCLevelWarning;
		break;
	case ESentryLevel::Error:
		nativeLevel = SentryObjCLevelError;
		break;
	case ESentryLevel::Fatal:
		nativeLevel = SentryObjCLevelFatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return nativeLevel;
}

SentryObjCLogLevel FAppleSentryConverters::SentryLogLevelToNative(ESentryLevel level)
{
	SentryObjCLogLevel nativeLevel = SentryObjCLogLevelDebug;

	switch (level)
	{
	case ESentryLevel::Debug:
		nativeLevel = SentryObjCLogLevelDebug;
		break;
	case ESentryLevel::Info:
		nativeLevel = SentryObjCLogLevelInfo;
		break;
	case ESentryLevel::Warning:
		nativeLevel = SentryObjCLogLevelWarn;
		break;
	case ESentryLevel::Error:
		nativeLevel = SentryObjCLogLevelError;
		break;
	case ESentryLevel::Fatal:
		nativeLevel = SentryObjCLogLevelFatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry level value used. Debug will be returned."));
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
		id object = VariantToNative(*it);
		if (object != nil)
		{
			[arr addObject:object];
		}
	}

	return arr;
}

NSDictionary* FAppleSentryConverters::VariantMapToNative(const TMap<FString, FSentryVariant>& variantMap)
{
	NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:variantMap.Num()];

	for (auto it = variantMap.CreateConstIterator(); it; ++it)
	{
		id object = VariantToNative(it.Value());
		if (object != nil)
		{
			[dict setValue:object forKey:it.Key().GetNSString()];
		}
	}

	return dict;
}

SentryObjCStacktrace* FAppleSentryConverters::CallstackToNative(const TArray<FProgramCounterSymbolInfo>& callstack)
{
	int32 framesCount = callstack.Num();

	NSMutableArray* arr = [NSMutableArray arrayWithCapacity:framesCount];

	for (int i = 0; i < framesCount; ++i)
	{
		SentryObjCFrame* frame = [[SENTRY_APPLE_CLASS(SentryObjCFrame) alloc] init];
		frame.instructionAddress = FString::Printf(TEXT("0x%llx"), callstack[framesCount - i - 1].ProgramCounter).GetNSString();
		[arr addObject:frame];
	}

	SentryObjCStacktrace* trace = [[SENTRY_APPLE_CLASS(SentryObjCStacktrace) alloc] initWithFrames:arr registers:@{}];

	return trace;
}

SentryObjCAttribute* FAppleSentryConverters::VariantToAttributeNative(const FSentryVariant& variant)
{
	switch (variant.GetType())
	{
	case ESentryVariantType::Integer:
		return [[SENTRY_APPLE_CLASS(SentryObjCAttribute) alloc] initWithInteger:variant.GetValue<int32>()];
	case ESentryVariantType::Float:
		return [[SENTRY_APPLE_CLASS(SentryObjCAttribute) alloc] initWithDouble:(double)variant.GetValue<float>()];
	case ESentryVariantType::Bool:
		return [[SENTRY_APPLE_CLASS(SentryObjCAttribute) alloc] initWithBoolean:variant.GetValue<bool>()];
	case ESentryVariantType::String:
		return [[SENTRY_APPLE_CLASS(SentryObjCAttribute) alloc] initWithString:variant.GetValue<FString>().GetNSString()];
	case ESentryVariantType::Array:
	{
		// Convert array to native and then to JSON string representation
		NSArray* nativeArray = VariantArrayToNative(variant.GetValue<TArray<FSentryVariant>>());
		NSError* error = nil;
		NSData* jsonData = [NSJSONSerialization dataWithJSONObject:nativeArray options:0 error:&error];
		if (jsonData && !error)
		{
			NSString* jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
			return [[SENTRY_APPLE_CLASS(SentryObjCAttribute) alloc] initWithString:jsonString];
		}
		else
		{
			return nil;
		}
	}
	case ESentryVariantType::Map:
	{
		// Convert map to native and then to JSON string representation
		NSDictionary* nativeDict = VariantMapToNative(variant.GetValue<TMap<FString, FSentryVariant>>());
		NSError* error = nil;
		NSData* jsonData = [NSJSONSerialization dataWithJSONObject:nativeDict options:0 error:&error];
		if (jsonData && !error)
		{
			NSString* jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
			return [[SENTRY_APPLE_CLASS(SentryObjCAttribute) alloc] initWithString:jsonString];
		}
		else
		{
			return nil;
		}
	}
	default:
		return nil;
	}
}

SentryObjCAttributeContent* FAppleSentryConverters::VariantToAttributeContentNative(const FSentryVariant& variant)
{
	switch (variant.GetType())
	{
	case ESentryVariantType::Integer:
		return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) integer:variant.GetValue<int32>()];
	case ESentryVariantType::Float:
		return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) double:(double)variant.GetValue<float>()];
	case ESentryVariantType::Bool:
		return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) boolean:variant.GetValue<bool>()];
	case ESentryVariantType::String:
		return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) string:variant.GetValue<FString>().GetNSString()];
	case ESentryVariantType::Array:
	{
		const TArray<FSentryVariant>& arr = variant.GetValue<TArray<FSentryVariant>>();
		// Check if array is homogeneous and convert to typed array if possible
		if (arr.Num() > 0)
		{
			ESentryVariantType firstType = arr[0].GetType();
			bool isHomogeneous = true;
			for (const FSentryVariant& item : arr)
			{
				if (item.GetType() != firstType)
				{
					isHomogeneous = false;
					break;
				}
			}

			if (isHomogeneous)
			{
				switch (firstType)
				{
				case ESentryVariantType::String:
				{
					NSMutableArray<NSString*>* stringArr = [NSMutableArray arrayWithCapacity:arr.Num()];
					for (const FSentryVariant& item : arr)
					{
						[stringArr addObject:item.GetValue<FString>().GetNSString()];
					}
					return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) stringArray:stringArr];
				}
				case ESentryVariantType::Integer:
				{
					NSMutableArray<NSNumber*>* intArr = [NSMutableArray arrayWithCapacity:arr.Num()];
					for (const FSentryVariant& item : arr)
					{
						[intArr addObject:@(item.GetValue<int32>())];
					}
					return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) integerArray:intArr];
				}
				case ESentryVariantType::Float:
				{
					NSMutableArray<NSNumber*>* doubleArr = [NSMutableArray arrayWithCapacity:arr.Num()];
					for (const FSentryVariant& item : arr)
					{
						[doubleArr addObject:@((double)item.GetValue<float>())];
					}
					return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) doubleArray:doubleArr];
				}
				case ESentryVariantType::Bool:
				{
					NSMutableArray<NSNumber*>* boolArr = [NSMutableArray arrayWithCapacity:arr.Num()];
					for (const FSentryVariant& item : arr)
					{
						[boolArr addObject:@(item.GetValue<bool>())];
					}
					return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) booleanArray:boolArr];
				}
				default:
					break;
				}
			}
		}
		// Fall back to JSON string for mixed arrays
		NSArray* nativeArray = VariantArrayToNative(arr);
		NSError* error = nil;
		NSData* jsonData = [NSJSONSerialization dataWithJSONObject:nativeArray options:0 error:&error];
		if (jsonData && !error)
		{
			NSString* jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
			return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) string:jsonString];
		}
		return nil;
	}
	case ESentryVariantType::Map:
	{
		// Convert map to JSON string
		NSDictionary* nativeDict = VariantMapToNative(variant.GetValue<TMap<FString, FSentryVariant>>());
		NSError* error = nil;
		NSData* jsonData = [NSJSONSerialization dataWithJSONObject:nativeDict options:0 error:&error];
		if (jsonData && !error)
		{
			NSString* jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
			return [SENTRY_APPLE_CLASS(SentryObjCAttributeContent) string:jsonString];
		}
		return nil;
	}
	default:
		return nil;
	}
}

NSDictionary<NSString*, SentryObjCAttributeContent*>* FAppleSentryConverters::VariantMapToAttributeContentNative(const TMap<FString, FSentryVariant>& variantMap)
{
	NSMutableDictionary<NSString*, SentryObjCAttributeContent*>* dict = [NSMutableDictionary dictionaryWithCapacity:variantMap.Num()];

	for (auto it = variantMap.CreateConstIterator(); it; ++it)
	{
		SentryObjCAttributeContent* content = VariantToAttributeContentNative(it.Value());
		if (content != nil)
		{
			[dict setObject:content forKey:it.Key().GetNSString()];
		}
	}

	return dict;
}

ESentryLevel FAppleSentryConverters::SentryLevelToUnreal(SentryObjCLevel level)
{
	ESentryLevel unrealLevel = ESentryLevel::Debug;

	switch (level)
	{
	case SentryObjCLevelDebug:
		unrealLevel = ESentryLevel::Debug;
		break;
	case SentryObjCLevelInfo:
		unrealLevel = ESentryLevel::Info;
		break;
	case SentryObjCLevelWarning:
		unrealLevel = ESentryLevel::Warning;
		break;
	case SentryObjCLevelError:
		unrealLevel = ESentryLevel::Error;
		break;
	case SentryObjCLevelFatal:
		unrealLevel = ESentryLevel::Fatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry level value used. Debug will be returned."));
	}

	return unrealLevel;
}

ESentryLevel FAppleSentryConverters::SentryLogLevelToUnreal(SentryObjCLogLevel level)
{
	ESentryLevel unrealLevel = ESentryLevel::Debug;

	switch (level)
	{
	case SentryObjCLogLevelTrace:
	case SentryObjCLogLevelDebug:
		unrealLevel = ESentryLevel::Debug;
		break;
	case SentryObjCLogLevelInfo:
		unrealLevel = ESentryLevel::Info;
		break;
	case SentryObjCLogLevelWarn:
		unrealLevel = ESentryLevel::Warning;
		break;
	case SentryObjCLogLevelError:
		unrealLevel = ESentryLevel::Error;
		break;
	case SentryObjCLogLevelFatal:
		unrealLevel = ESentryLevel::Fatal;
		break;
	default:
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown Sentry structured log level value used. Debug will be returned."));
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
	if (variant == nil)
	{
		return FSentryVariant();
	}
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
		if (strcmp(objCType, @encode(double)) == 0)
		{
			return FSentryVariant(static_cast<float>([number doubleValue]));
		}
		if (strcmp(objCType, @encode(int)) == 0)
		{
			return FSentryVariant([number intValue]);
		}
		if (strcmp(objCType, @encode(long)) == 0 || strcmp(objCType, @encode(long long)) == 0)
		{
			return FSentryVariant(static_cast<int32>([number longLongValue]));
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

SentryObjCLevel FAppleSentryConverters::StringToSentryLevel(NSString* string)
{
	SentryObjCLevel nativeLevel = SentryObjCLevelDebug;

	if ([string isEqualToString:@"debug"])
	{
		nativeLevel = SentryObjCLevelDebug;
	}
	else if ([string isEqualToString:@"info"])
	{
		nativeLevel = SentryObjCLevelInfo;
	}
	else if ([string isEqualToString:@"warning"])
	{
		nativeLevel = SentryObjCLevelWarning;
	}
	else if ([string isEqualToString:@"error"])
	{
		nativeLevel = SentryObjCLevelError;
	}
	else if ([string isEqualToString:@"fatal"])
	{
		nativeLevel = SentryObjCLevelFatal;
	}
	else
	{
		UE_LOG(LogSentrySdk, Warning, TEXT("Unknown sentry level value used. Debug will be returned."));
	}

	return nativeLevel;
}

FSentryVariant FAppleSentryConverters::SentryAttributeToVariant(SentryObjCAttribute* attribute)
{
	if (!attribute)
	{
		return FSentryVariant();
	}

	NSString* type = attribute.type;

	if ([type isEqualToString:@"string"])
	{
		NSString* value = (NSString*)attribute.value;
		return FSentryVariant(FString(value));
	}
	else if ([type isEqualToString:@"boolean"])
	{
		NSNumber* value = (NSNumber*)attribute.value;
		return FSentryVariant([value boolValue]);
	}
	else if ([type isEqualToString:@"integer"])
	{
		NSNumber* value = (NSNumber*)attribute.value;
		return FSentryVariant([value intValue]);
	}
	else if ([type isEqualToString:@"double"])
	{
		NSNumber* value = (NSNumber*)attribute.value;
		return FSentryVariant((float)[value doubleValue]);
	}
	else
	{
		return FSentryVariant(FString([attribute.value description]));
	}
}

FSentryVariant FAppleSentryConverters::SentryAttributeContentToVariant(SentryObjCAttributeContent* content)
{
	if (!content)
	{
		return FSentryVariant();
	}

	NSString* type = content.type;

	if ([type isEqualToString:@"string"])
	{
		return FSentryVariant(FString((NSString*)content.value));
	}
	else if ([type isEqualToString:@"boolean"])
	{
		return FSentryVariant([(NSNumber*)content.value boolValue]);
	}
	else if ([type isEqualToString:@"integer"])
	{
		return FSentryVariant(static_cast<int32>([(NSNumber*)content.value integerValue]));
	}
	else if ([type isEqualToString:@"double"])
	{
		return FSentryVariant(static_cast<float>([(NSNumber*)content.value doubleValue]));
	}
	else if ([type isEqualToString:@"string[]"]
		|| [type isEqualToString:@"integer[]"]
		|| [type isEqualToString:@"double[]"]
		|| [type isEqualToString:@"boolean[]"])
	{
		return FSentryVariant(VariantArrayToUnreal((NSArray*)content.value));
	}

	return FSentryVariant();
}

#endif // !USE_SENTRY_NATIVE
