// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryBeforeBreadcrumbHandler.h"

#include "SentryBreadcrumb.h"
#include "SentryHint.h"

USentryBreadcrumb* USentryBeforeBreadcrumbHandler::HandleBeforeBreadcrumb_Implementation(USentryBreadcrumb* Breadcrumb, USentryHint* Hint)
{
	return Breadcrumb;
}
