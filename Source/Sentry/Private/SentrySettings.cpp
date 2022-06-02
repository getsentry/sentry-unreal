// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentrySettings.h"

USentrySettings::USentrySettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InitAutomatically(false)
	, bOnMapLoadingStarted(false)
	, bOnMapLoaded(false)
	, bOnGameStateClassChanged(false)
	, bOnGameSessionIDChanged(false)
	, bOnUserActivityStringChanged(false)
{
	DsnUrl = TEXT("");
}
