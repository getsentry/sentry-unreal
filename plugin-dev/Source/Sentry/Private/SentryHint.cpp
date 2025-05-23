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
