// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Android/AndroidJNI.h"

enum class ESentryJavaClassType : uint8
{
	System = 0,
	External
};

struct FSentryJavaClass
{
	FName Name;
	ESentryJavaClassType Type;
};

struct FSentryJavaMethod
{
	FName Name;
	FName Signature;
	jmethodID Method;
	bool IsStatic;
};