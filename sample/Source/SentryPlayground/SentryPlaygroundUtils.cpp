#include "SentryPlaygroundUtils.h"

void USentryPlaygroundUtils::Crash()
{
	volatile char *ptr = 0;
	*ptr += 1;
}

void USentryPlaygroundUtils::Assert()
{
	char *ptr = nullptr;
	check(ptr != nullptr);
}
