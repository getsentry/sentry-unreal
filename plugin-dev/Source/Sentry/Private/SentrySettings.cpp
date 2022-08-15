// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettings.h"

USentrySettings::USentrySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InitAutomatically(false)
	, UploadSymbolsAutomatically(false)
{
	DsnUrl = TEXT("");
	Release = TEXT("");
	ProjectName = TEXT("");
	OrganisationName = TEXT("");
	AuthToken = TEXT("");
}
