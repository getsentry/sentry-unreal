// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryDsnUrl.h"

#if USE_SENTRY_NATIVE

SentryDsnUrl::SentryDsnUrl(const FString& Dsn)
{
	FString UrlRemainder;

	Dsn.Split(TEXT("://"), &Scheme, &UrlRemainder);
	UrlRemainder.Split(TEXT("@"), &Key, &UrlRemainder);
	UrlRemainder.Split(TEXT("/"), &Host, &ProjectId);
}

const FString& SentryDsnUrl::GetScheme() const
{
	return Scheme;
}

const FString& SentryDsnUrl::GetHost() const
{
	return Host;
}

const FString& SentryDsnUrl::GetProjectId() const
{
	return ProjectId;
}

const FString& SentryDsnUrl::GetKey() const
{
	return Key;
}

#endif