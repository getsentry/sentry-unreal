// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentrySDK.h"

void USentrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	USentrySettings* Settings = FSentrySDKModule::Get().GetSettings();

	Initialize(Settings);
}

void USentrySubsystem::Initialize(USentrySettings* sentrySettings)
{
#if PLATFORM_ANDROID
	// TODO
#elif PLATFORM_IOS
	// TODO
#endif
}
