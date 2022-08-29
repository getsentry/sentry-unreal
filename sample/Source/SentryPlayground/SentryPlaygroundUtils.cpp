#include "SentryPlaygroundUtils.h"

void USentryPlaygroundUtils::Crash()
{
	// Supressing warnings for when using warnings as errors on the target. 
	#pragma warning(suppress: 6011)	
	char *ptr = 0;
	#pragma warning(suppress: 6011)
	*ptr += 1;
}

void USentryPlaygroundUtils::Assert()
{
	char *ptr = nullptr;
	check(ptr != nullptr);
}
