// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySubsystem.h"
#include "SentryModule.h"

void USentrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	USentrySettings* Settings = FSentryModule::Get().GetSettings();

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
