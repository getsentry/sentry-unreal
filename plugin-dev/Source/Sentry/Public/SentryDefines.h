// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSentrySdk, Verbose, All);

#define SENTRY_DEPRECATED(Message) [[deprecated(Message " Update your code to the new API before upgrading to the next release, otherwise your project will no longer compile.")]]
