// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettings.h"

USentrySettings::USentrySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DsnUrl = TEXT("");
}
