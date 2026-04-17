// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryPlaygroundCrashUtils.h"

#include "SentryGCCallback.h"

#include "SentryErrorOutputDevice.h"
#include "SentryEvent.h"

#include "CoreGlobals.h"
#include "Engine/Engine.h"
#include "HAL/PlatformProcess.h"
#include "UObject/UObjectGlobals.h"

#if PLATFORM_MICROSOFT
#include "Microsoft/WindowsHWrapper.h"
#endif


void USentryPlaygroundCrashUtils::Terminate(ESentryAppTerminationType Type)
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
				const size_t BlockSize = 1024 * 1024 * 1024;
				while (true)
				{
					void* _ = FMemory::Malloc(BlockSize);
				}
			}
			break;
		case ESentryAppTerminationType::MemoryCorruption:
			{
				void* ptr = FMemory::Malloc(1024);
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
		case ESentryAppTerminationType::AssertReentrant:
			{
				FSentryErrorOutputDevice* SentryErrorDevice = static_cast<FSentryErrorOutputDevice*>(GError);
				if (SentryErrorDevice)
				{
					SentryErrorDevice->OnAssert.AddLambda([](const FString& Message)
					{
						UE_LOG(LogTemp, Warning, TEXT("OnAssert handler called, triggering reentrant assert..."));

						char* reentrantPtr = nullptr;
						check(reentrantPtr != nullptr);
					});
				}

				char* initialPtr = nullptr;
				check(initialPtr != nullptr);
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
		case ESentryAppTerminationType::Hang:
			UE_LOG(LogTemp, Warning, TEXT("Triggering hang: blocking game thread for 10 seconds..."));
			FPlatformProcess::Sleep(10.0f);
			UE_LOG(LogTemp, Warning, TEXT("Hang period ended."));
			break;
		default:
			{
				UE_LOG(LogTemp, Warning, TEXT("Unknown app termination type!"));
			}
			break;
	}
}

void USentryPlaygroundCrashUtils::CaptureEventDuringGC()
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
