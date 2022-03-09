// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScope.h"

#if PLATFORM_IOS
#import <Sentry/Sentry.h>
#include "IOS/SentryConvertorsIOS.h"
#endif

void USentryScope::SetLevel(ESentryLevel level)
{
#if PLATFORM_IOS
	[_scopeIOS setLevel:SentryConvertorsIOS::SentryLevelToNative(level)];
#endif
}

#if PLATFORM_IOS
void USentryScope::InitWithNativeObjectIOS(SentryScope* scope)
{
	_scopeIOS = scope;
}
#endif
