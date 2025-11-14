// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "MicrosoftSentryFeedback.h"

#include "GenericPlatform/GenericPlatformSentryAttachment.h"

#if USE_SENTRY_NATIVE

void FMicrosoftSentryFeedback::AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment)
{
	sentry_attachment_t* nativeAttachment =
		sentry_feedback_hint_attach_filew(Hint, *attachment->GetPath());

	if (!attachment->GetFilename().IsEmpty())
		sentry_attachment_set_filenamew(nativeAttachment, *attachment->GetFilename());

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetNativeObject(nativeAttachment);
}

void FMicrosoftSentryFeedback::AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment)
{
	const TArray<uint8>& byteBuf = attachment->GetDataByRef();

	sentry_attachment_t* nativeAttachment =
		sentry_feedback_hint_attach_bytesw(Hint, reinterpret_cast<const char*>(byteBuf.GetData()), byteBuf.Num(), *attachment->GetFilename());

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetNativeObject(nativeAttachment);
}

#endif
