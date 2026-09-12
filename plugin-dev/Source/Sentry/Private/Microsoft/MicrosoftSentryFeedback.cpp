// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "MicrosoftSentryFeedback.h"

#include "GenericPlatform/GenericPlatformSentryAttachment.h"

#if USE_SENTRY_NATIVE

FMicrosoftSentryFeedback::FMicrosoftSentryFeedback()
	: FGenericPlatformSentryFeedback()
{
}

FMicrosoftSentryFeedback::FMicrosoftSentryFeedback(const FString& message)
	: FGenericPlatformSentryFeedback(message)
{
}

void FMicrosoftSentryFeedback::AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment)
{
	sentry_value_t nativeAttachment =
		sentry_attachment_from_filew(*attachment->GetPath());

	if (!attachment->GetFilename().IsEmpty())
		sentry_attachment_set_filenamew(nativeAttachment, *attachment->GetFilename());

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetUuid(sentry_hint_add_attachment(Hint, nativeAttachment));
}

void FMicrosoftSentryFeedback::AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment)
{
	const TArray<uint8>& byteBuf = attachment->GetDataByRef();

	sentry_value_t nativeAttachment =
		sentry_attachment_from_bytesw(reinterpret_cast<const char*>(byteBuf.GetData()), byteBuf.Num(), *attachment->GetFilename());

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetUuid(sentry_hint_add_attachment(Hint, nativeAttachment));
}

#endif
