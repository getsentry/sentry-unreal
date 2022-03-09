// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentrySettings.h"
#include "SentryDataTypes.h"
#include "SentryScope.h"

class SentryIOS
{
public:
	static void InitWithSettings(const USentrySettings* settings);
	static FGuid CaptureMessage(const FString& message, ESentryLevel level);
	static FGuid CaptureMessage(const FString& message, const FConfigureScopeDelegate& onScopeConfigure, ESentryLevel level);
	static FGuid CaptureError();
};
