// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryFeedback.h"

#include "AndroidSentryId.h"

#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryFeedback::FAndroidSentryFeedback(const FString& message)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Feedback, "(Ljava/lang/String;)V",
		  *GetJString(message))
{
	SetupClassMethods();
}

void FAndroidSentryFeedback::SetupClassMethods()
{
	GetMessageMethod = GetMethod("getMessage", "()Ljava/lang/String;");
	SetNameMethod = GetMethod("setName", "(Ljava/lang/String;)V");
	GetNameMethod = GetMethod("getName", "()Ljava/lang/String;");
	SetContactEmailMethod = GetMethod("setContactEmail", "(Ljava/lang/String;)V");
	GetContactEmailMethod = GetMethod("getContactEmail", "()Ljava/lang/String;");
	SetAssociatedEventMethod = GetMethod("setAssociatedEventId", "(Lio/sentry/protocol/SentryId;)V");
	GetAssociatedEventMethod = GetMethod("getAssociatedEventId", "()Lio/sentry/protocol/SentryId;");
}

FString FAndroidSentryFeedback::GetMessage() const
{
	return CallMethod<FString>(GetMessageMethod);
}

void FAndroidSentryFeedback::SetName(const FString& name)
{
	CallMethod<void>(SetNameMethod, *GetJString(name));
}

FString FAndroidSentryFeedback::GetName() const
{
	return CallMethod<FString>(GetNameMethod);
}

void FAndroidSentryFeedback::SetContactEmail(const FString& email)
{
	CallMethod<void>(SetContactEmailMethod, *GetJString(email));
}

FString FAndroidSentryFeedback::GetContactEmail() const
{
	return CallMethod<FString>(GetContactEmailMethod);
}

void FAndroidSentryFeedback::SetAssociatedEvent(const FString& eventId)
{
	if (eventId.IsEmpty())
		return;

	TSharedPtr<FAndroidSentryId> idAndroid = MakeShareable(new FAndroidSentryId(eventId));
	CallMethod<void>(SetAssociatedEventMethod, idAndroid->GetJObject());
}

FString FAndroidSentryFeedback::GetAssociatedEvent() const
{
	auto idAndroid = CallObjectMethod<jobject>(GetAssociatedEventMethod);
	if (!idAndroid)
	{
		return FString();
	}

	TSharedPtr<FAndroidSentryId> eventId = MakeShareable(new FAndroidSentryId(*idAndroid));
	return eventId->ToString();
}
