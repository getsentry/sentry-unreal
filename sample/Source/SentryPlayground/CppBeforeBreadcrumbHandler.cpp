// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "CppBeforeBreadcrumbHandler.h"

#include "SentryBreadcrumb.h"
#include "SentryHint.h"
#include "SentryVariant.h"

USentryBreadcrumb* UCppBeforeBreadcrumbHandler::HandleBeforeBreadcrumb_Implementation(
	USentryBreadcrumb* Breadcrumb, USentryHint* Hint)
{
	FString Message = Breadcrumb->GetMessage();

	// Discard this specific breadcrumb
	if (Message == TEXT("Breadcrumb to be discarded"))
	{
		return nullptr;
	}

	// Modify this specific breadcrumb by adding data
	if (Message == TEXT("Breadcrumb to be modified"))
	{
		TMap<FString, FSentryVariant> Data;
		Data.Add(TEXT("handler_key"), FSentryVariant(TEXT("handler_value")));
		Breadcrumb->SetData(Data);
	}

	return Super::HandleBeforeBreadcrumb_Implementation(Breadcrumb, Hint);
}
