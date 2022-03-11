// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryLibrary.h"

void USentryLibrary::Crash()
{
	char *ptr = 0;
	*ptr += 1;
}

void USentryLibrary::Assert()
{
	char *ptr = nullptr;
	check(ptr != nullptr);
}
