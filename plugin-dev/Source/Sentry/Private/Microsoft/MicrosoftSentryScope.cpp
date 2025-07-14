// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "MicrosoftSentryScope.h"

#include "GenericPlatform/GenericPlatformSentryAttachment.h"

#if USE_SENTRY_NATIVE

void FMicrosoftSentryScope::AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment, sentry_scope_t* scope)
{
	sentry_attachment_t* nativeAttachment =
		sentry_scope_attach_filew(scope, *attachment->GetPath());

	if (!attachment->GetFilename().IsEmpty())
		sentry_attachment_set_filenamew(nativeAttachment, *attachment->GetFilename());

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetNativeObject(nativeAttachment);
}

void FMicrosoftSentryScope::AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment, sentry_scope_t* scope)
{
	const TArray<uint8>& byteBuf = attachment->GetDataByRef();

	sentry_attachment_t* nativeAttachment =
		sentry_scope_attach_bytesw(scope, reinterpret_cast<const char*>(byteBuf.GetData()), byteBuf.Num(), *attachment->GetFilename());

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetNativeObject(nativeAttachment);
}

#endif
