// Copyright (c) 2023 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if USE_SENTRY_NATIVE

class FSentryDsnUrl
{
public:
	FSentryDsnUrl(const FString& Dsn)
	{
		FString UrlRemainder;

		Dsn.Split(TEXT("://"), &Scheme, &UrlRemainder);
		UrlRemainder.Split(TEXT("@"), &Key, &UrlRemainder);
		UrlRemainder.Split(TEXT("/"), &Host, &ProjectId);
	}

	const FString& GetScheme() const
	{
		return Scheme;
	}

	const FString& GetHost() const
	{
		return Host;
	}

	const FString& GetProjectId() const
	{
		return ProjectId;
	}

	const FString& GetKey() const
	{
		return Key;
	}

private:
	FString Scheme;
	FString Key;
	FString Host;
	FString ProjectId;
};

#endif // USE_SENTRY_NATIVE
