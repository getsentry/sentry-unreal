// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "AppleSentryHint.h"

#if !USE_SENTRY_NATIVE

#include "AppleSentryAttachment.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryHint::FAppleSentryHint()
{
	HintApple = [[SENTRY_APPLE_CLASS(SentryObjCHint) alloc] init];
}

FAppleSentryHint::FAppleSentryHint(SentryObjCHint* hint)
{
	HintApple = hint;
}

FAppleSentryHint::~FAppleSentryHint()
{
	// Put custom destructor logic here if needed
}

SentryObjCHint* FAppleSentryHint::GetNativeObject()
{
	return HintApple;
}

void FAppleSentryHint::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FAppleSentryAttachment> attachmentApple = StaticCastSharedPtr<FAppleSentryAttachment>(attachment);
	if (!attachmentApple)
	{
		return;
	}

	SentryObjCAttachment* nativeAttachment = attachmentApple->GetNativeObject();
	if (nativeAttachment == nil)
	{
		return;
	}

	HintApple.attachments = [HintApple.attachments arrayByAddingObject:nativeAttachment];
}

#endif // !USE_SENTRY_NATIVE
