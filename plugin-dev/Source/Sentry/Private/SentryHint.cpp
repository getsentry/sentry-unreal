// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryHint.h"

#include "SentryAttachment.h"
#include "Interface/SentryHintInterface.h"

void USentryHint::AddAttachment(USentryAttachment* Attachment)
{
	if(!NativeImpl)
		return;

	NativeImpl->AddAttachment(Attachment->GetNativeObject());
}
