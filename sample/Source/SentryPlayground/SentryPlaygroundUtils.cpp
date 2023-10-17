#include "SentryPlaygroundUtils.h"

void USentryPlaygroundUtils::Crash()
{
	UE_LOG(LogTemp, Log, TEXT("USentryPlaygroundUtils::Crash() utility function called!"));

	// Supressing warnings for when using warnings as errors on the target. 
	#pragma warning(suppress: 6011)	
	char *ptr = 0;
	#pragma warning(suppress: 6011)
	*ptr += 1;
}

void USentryPlaygroundUtils::Assert()
{
	UE_LOG(LogTemp, Log, TEXT("USentryPlaygroundUtils::Assert() utility function called!"));

	char *ptr = nullptr;
	check(ptr != nullptr);
}
