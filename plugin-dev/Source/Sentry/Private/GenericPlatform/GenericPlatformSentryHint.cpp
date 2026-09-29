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

int32 FGenericPlatformSentryHint::RemoveAttachments(const FString& filenamePattern)
{
	if (!Hint)
	{
		return 0;
	}

	sentry_value_t attachments = sentry_hint_get_attachments(Hint);

	TArray<sentry_uuid_t> matchedIds;
	for (size_t i = 0; i < sentry_value_get_length(attachments); ++i)
	{
		sentry_value_t attachment = sentry_value_get_by_index(attachments, i);

		const FString filename = UTF8_TO_TCHAR(sentry_value_as_string(sentry_value_get_by_key(attachment, "filename")));
		if (filename.MatchesWildcard(filenamePattern))
		{
			matchedIds.Add(sentry_uuid_from_string(sentry_value_as_string(sentry_value_get_by_key(attachment, "id"))));
		}
	}

	for (const sentry_uuid_t& id : matchedIds)
	{
		sentry_hint_remove_attachment(Hint, id);
	}

	return matchedIds.Num();
}

void FGenericPlatformSentryHint::ClearAttachments()
{
	if (!Hint)
	{
		return;
	}

	sentry_hint_clear_attachments(Hint);
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
