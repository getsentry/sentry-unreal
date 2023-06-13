// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if USE_SENTRY_NATIVE

class SentryDsnUrl
{
public:
	SentryDsnUrl(const FString& Dsn);

	const FString& GetScheme() const;
	const FString& GetHost() const;
	const FString& GetProjectId() const;
	const FString& GetKey() const;

private:
	FString Scheme;
	FString Key;
	FString Host;
	FString ProjectId;
};

#endif