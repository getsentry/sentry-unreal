// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SentrySettings.h"
#include "SentryDataTypes.h"

class SentryIOS
{
public:
	static void InitWithSettings(const USentrySettings* settings);
	static void CaptureMessage(const FString& message, ESentryLevel level);
	static void CaptureError();
};
