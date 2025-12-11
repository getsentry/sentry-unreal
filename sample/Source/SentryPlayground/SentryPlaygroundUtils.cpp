// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlaygroundUtils.h"

#include "Engine/Engine.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectGlobals.h"
#include "Async/Async.h"
#include "SentrySubsystem.h"
#include "SentryEvent.h"
#include "SentryGCCallback.h"

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
		case ESentryAppTerminationType::StackOverflow:
			char buffer[4096];
			FMemory::Memset((void*)buffer, 0xAA, sizeof(buffer));
			UE_LOG(LogTemp, VeryVerbose, TEXT("Stack addr: %p"), &buffer);
			Terminate(Type);
			break;
		case ESentryAppTerminationType::OutOfMemory:
			{
				size_t Count = 1024;
				while (true)
				{
					void* _ = FMemory::Malloc(Count);
					Count *= 2;
				}
			}
			break;
		case ESentryAppTerminationType::MemoryCorruption:
			{
				void* ptr = FMemory::Malloc(100);
				FMemory::Free(ptr);
				FMemory::Free(ptr);
			}
			break;
		case ESentryAppTerminationType::RenderThreadCrash:
			GEngine->Exec(nullptr, TEXT("Debug RenderCrash"));
			break;
		case ESentryAppTerminationType::GpuDebugCrash:
			GEngine->Exec(nullptr, TEXT("GPUDebugCrash platformbreak"));
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
		case ESentryAppTerminationType::Fatal:
			{
				UE_LOG(LogTemp, Fatal, TEXT("Fatal error!"));
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


void USentryPlaygroundUtils::CaptureEventDuringGC()
{
	FSentryGCCallback* GCCallback = new FSentryGCCallback();

	TSharedRef<FDelegateHandle> HandlePtr = MakeShared<FDelegateHandle>();
	*HandlePtr = FCoreUObjectDelegates::GetPostGarbageCollect().AddLambda([GCCallback, HandlePtr]() {
		delete GCCallback;
		FCoreUObjectDelegates::GetPostGarbageCollect().Remove(*HandlePtr);
	});

	TArray<UObject*> ObjectsToGC;
	for (int32 i = 0; i < 100; ++i)
	{
		USentryEvent* TestObj = NewObject<USentryEvent>();
		ObjectsToGC.Add(TestObj);
	}

	// Clear references to make objects eligible for GC
	ObjectsToGC.Empty();

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

