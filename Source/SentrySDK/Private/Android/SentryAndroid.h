// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SentrySettings.h"

class SentryAndroid
{
public:
	static void InitWithSettings(const USentrySettings* settings);

private:
	static const ANSICHAR* SentryJavaClassName;
};
