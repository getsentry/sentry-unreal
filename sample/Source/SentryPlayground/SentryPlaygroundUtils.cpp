// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlaygroundUtils.h"

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if PLATFORM_MICROSOFT
#include "Microsoft/WindowsHWrapper.h"
#endif

void USentryPlaygroundUtils::Terminate(ESentryAppTerminationType Type)
{
	switch (Type)
	{
		case ESentryAppTerminationType::NullPointer:
			{
				volatile char *ptr = nullptr;
				*ptr += 1;
			}
			break;
		case ESentryAppTerminationType::ArrayOutOfBounds:
			{
				TArray<int32> emptyArray;
				emptyArray[0] = 10;
			}
			break;
		case ESentryAppTerminationType::BadFunctionPtr:
			{
				void(*funcPointer)() = nullptr;
				funcPointer();
			}
			break;
		case ESentryAppTerminationType::InvalidMemoryAccess:
			{
				int* addrPtr = reinterpret_cast<int*>(0x12345678);
				*addrPtr = 10;
			}
			break;
		case ESentryAppTerminationType::FastFail:
			{
#if PLATFORM_MICROSOFT
				RaiseFailFastException(nullptr, nullptr, 0);
#endif
			}
			break;
		case ESentryAppTerminationType::Assert:
			{
				char *assertPtr = nullptr;
				check(assertPtr != nullptr);
			}
			break;
		case ESentryAppTerminationType::Ensure:
			{
				char *ensurePtr = nullptr;
				ensure(ensurePtr != nullptr);
			}
		break;
		default:
			{
				UE_LOG(LogTemp, Warning, TEXT("Uknown app termination type!"));
			}
			break;
	}
}

TArray<uint8> USentryPlaygroundUtils::StringToBytesArray(const FString& InString)
{
	TArray<uint8> byteArray;
	byteArray.AddUninitialized(InString.Len());

	uint8* byteArrayPtr = byteArray.GetData();

	int32 NumBytes = 0;
	const TCHAR* CharPos = *InString;

	while (*CharPos && NumBytes < TNumericLimits<int16>::Max())
	{
		byteArrayPtr[NumBytes] = (int8)(*CharPos);
		CharPos++;
		++NumBytes;
	}

	return byteArray;
}

FString USentryPlaygroundUtils::ByteArrayToString(const TArray<uint8>& Array)
{
	return BytesToString(Array.GetData(), Array.Num());
}

FString USentryPlaygroundUtils::SaveStringToFile(const FString& InString, const FString& Filename)
{
	FString filePath = FPaths::Combine(FPaths::ProjectPersistentDownloadDir(), Filename);

	FFileHelper::SaveStringToFile(InString, *filePath);

	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*filePath);
}

