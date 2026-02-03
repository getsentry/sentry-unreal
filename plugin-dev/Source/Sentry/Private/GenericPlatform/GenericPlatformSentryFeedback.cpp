// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "GenericPlatformSentryFeedback.h"
#include "GenericPlatformSentryAttachment.h"

#include "Infrastructure/GenericPlatformSentryConverters.h"

#if USE_SENTRY_NATIVE

FGenericPlatformSentryFeedback::FGenericPlatformSentryFeedback()
	: Hint(nullptr)
{
	Feedback = sentry_value_new_object();
}

FGenericPlatformSentryFeedback::FGenericPlatformSentryFeedback(const FString& message)
	: Hint(nullptr)
{
	Feedback = sentry_value_new_object();
	sentry_value_set_by_key(Feedback, "message", sentry_value_new_string(TCHAR_TO_UTF8(*message)));
}

FGenericPlatformSentryFeedback::~FGenericPlatformSentryFeedback()
{
	// Put custom destructor logic here if needed
}

sentry_value_t FGenericPlatformSentryFeedback::GetNativeObject()
{
	return Feedback;
}

FString FGenericPlatformSentryFeedback::GetMessage() const
{
	sentry_value_t message = sentry_value_get_by_key(Feedback, "message");
	return FString(UTF8_TO_TCHAR(sentry_value_as_string(message)));
}

void FGenericPlatformSentryFeedback::SetName(const FString& name)
{
	sentry_value_set_by_key(Feedback, "name", sentry_value_new_string(TCHAR_TO_UTF8(*name)));
}

FString FGenericPlatformSentryFeedback::GetName() const
{
	sentry_value_t username = sentry_value_get_by_key(Feedback, "name");
	return FString(UTF8_TO_TCHAR(sentry_value_as_string(username)));
}

void FGenericPlatformSentryFeedback::SetContactEmail(const FString& email)
{
	sentry_value_set_by_key(Feedback, "contact_email", sentry_value_new_string(TCHAR_TO_UTF8(*email)));
}

FString FGenericPlatformSentryFeedback::GetContactEmail() const
{
	sentry_value_t email = sentry_value_get_by_key(Feedback, "contact_email");
	return FString(UTF8_TO_TCHAR(sentry_value_as_string(email)));
}

void FGenericPlatformSentryFeedback::SetAssociatedEvent(const FString& eventId)
{
	if (eventId.IsEmpty())
		return;

	sentry_value_set_by_key(Feedback, "associated_event_id", sentry_value_new_string(TCHAR_TO_UTF8(*eventId)));
}

FString FGenericPlatformSentryFeedback::GetAssociatedEvent() const
{
	sentry_value_t comment = sentry_value_get_by_key(Feedback, "associated_event_id");
	return FString(UTF8_TO_TCHAR(sentry_value_as_string(comment)));
}

void FGenericPlatformSentryFeedback::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	if (!Hint)
	{
		Hint = sentry_hint_new();
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

sentry_hint_t* FGenericPlatformSentryFeedback::GetHintNativeObject()
{
	return Hint;
}

void FGenericPlatformSentryFeedback::AddFileAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment)
{
	sentry_attachment_t* nativeAttachment =
		sentry_hint_attach_file(Hint, TCHAR_TO_UTF8(*attachment->GetPath()));

	if (!attachment->GetFilename().IsEmpty())
		sentry_attachment_set_filename(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetFilename()));

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetNativeObject(nativeAttachment);
}

void FGenericPlatformSentryFeedback::AddByteAttachment(TSharedPtr<FGenericPlatformSentryAttachment> attachment)
{
	const TArray<uint8>& byteBuf = attachment->GetDataByRef();

	sentry_attachment_t* nativeAttachment =
		sentry_hint_attach_bytes(Hint, reinterpret_cast<const char*>(byteBuf.GetData()), byteBuf.Num(), TCHAR_TO_UTF8(*attachment->GetFilename()));

	if (!attachment->GetContentType().IsEmpty())
		sentry_attachment_set_content_type(nativeAttachment, TCHAR_TO_UTF8(*attachment->GetContentType()));

	attachment->SetNativeObject(nativeAttachment);
}

#endif
