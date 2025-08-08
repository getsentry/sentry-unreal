// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryGCCallback.h"

#include "SentrySubsystem.h"

#include "UObject/GarbageCollection.h"
#include "Engine/Engine.h"

void FSentryGCCallback::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (IsGarbageCollecting())
	{
		// This should trigger the Sentry's beforeSend callback invocation (if configured)
		// and skip its handler due to current GC limitations
		USentrySubsystem* SentrySubsystem = GEngine->GetEngineSubsystem<USentrySubsystem>();
		SentrySubsystem->CaptureMessage(TEXT("Message captured from within GC callback"), ESentryLevel::Error);
	}
}

FString FSentryGCCallback::GetReferencerName() const
{
	return TEXT("SentryGCCallback");
}