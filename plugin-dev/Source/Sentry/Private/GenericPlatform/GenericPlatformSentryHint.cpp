// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "GenericPlatformSentryHint.h"

#include "GenericPlatformSentryAttachment.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryHint::FGenericPlatformSentryHint()
	: Hint(nullptr)
{
}

FGenericPlatformSentryHint::FGenericPlatformSentryHint(sentry_hint_t* hint)
	: Hint(hint)
{
}

void FGenericPlatformSentryHint::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	if (!Hint || !attachment)
	{
		return;
	}

	TSharedPtr<FGenericPlatformSentryAttachment> platformAttachment = StaticCastSharedPtr<FGenericPlatformSentryAttachment>(attachment);

	if (!platformAttachment->GetPath().IsEmpty())
	{
		AddFileAttachment(platformAttachment);
	}
	else
	{
		AddByteAttachment(platformAttachment);
	}
}

void FGenericPlatformSentryHint::AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment)
{
	sentry_value_t nativeAttachment =
		sentry_attachment_from_file(TCHAR_TO_UTF8(*attachment->GetPath()));

	if (!attachment->GetFilename().IsEmpty())
		sentry_attachment_set_filename(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetFilename()));

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetUuid(sentry_hint_add_attachment(Hint, nativeAttachment));
}

void FGenericPlatformSentryHint::AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment)
{
	const TArray<uint8>& byteBuf = attachment->GetDataByRef();

	sentry_value_t nativeAttachment =
		sentry_attachment_from_bytes(reinterpret_cast<const char*>(byteBuf.GetData()), byteBuf.Num(), TCHAR_TO_UTF8(*attachment->GetFilename()));

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetUuid(sentry_hint_add_attachment(Hint, nativeAttachment));
}

#endif
