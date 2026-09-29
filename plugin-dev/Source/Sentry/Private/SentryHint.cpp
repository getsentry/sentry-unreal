// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryHint.h"

#include "HAL/PlatformSentryHint.h"
#include "SentryAttachment.h"

void USentryHint::Initialize()
{
	NativeImpl = CreateSharedSentryHint();
}

void USentryHint::AddAttachment(USentryAttachment* Attachment)
{
	if (!NativeImpl)
		return;

	NativeImpl->AddAttachment(Attachment->GetNativeObject());
}

int32 USentryHint::RemoveAttachments(const FString& FilenamePattern)
{
	if (!NativeImpl)
		return 0;

	return NativeImpl->RemoveAttachments(FilenamePattern);
}

void USentryHint::ClearAttachments()
{
	if (!NativeImpl)
		return;

	NativeImpl->ClearAttachments();
}
