#include "SentryPlaygroundUtils.h"

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
