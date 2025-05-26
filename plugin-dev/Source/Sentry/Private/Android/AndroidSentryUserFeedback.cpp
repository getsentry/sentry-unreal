// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryUserFeedback.h"

#include "AndroidSentryId.h"

#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryUserFeedback::FAndroidSentryUserFeedback(TSharedPtr<ISentryId> eventId)
	: FSentryJavaObjectWrapper(SentryJavaClasses::UserFeedback, "(Lio/sentry/protocol/SentryId;)V",
		  StaticCastSharedPtr<FAndroidSentryId>(eventId)->GetJObject())
{
	SetupClassMethods();
}

void FAndroidSentryUserFeedback::SetupClassMethods()
{
	SetNameMethod = GetMethod("setName", "(Ljava/lang/String;)V");
	GetNameMethod = GetMethod("getName", "()Ljava/lang/String;");
	SetEmailMethod = GetMethod("setEmail", "(Ljava/lang/String;)V");
	GetEmailMethod = GetMethod("getEmail", "()Ljava/lang/String;");
	SetCommentMethod = GetMethod("setComments", "(Ljava/lang/String;)V");
	GetCommentMethod = GetMethod("getComments", "()Ljava/lang/String;");
}

void FAndroidSentryUserFeedback::SetName(const FString& name)
{
	CallMethod<void>(SetNameMethod, *GetJString(name));
}

FString FAndroidSentryUserFeedback::GetName() const
{
	return CallMethod<FString>(GetNameMethod);
}

void FAndroidSentryUserFeedback::SetEmail(const FString& email)
{
	CallMethod<void>(SetEmailMethod, *GetJString(email));
}

FString FAndroidSentryUserFeedback::GetEmail() const
{
	return CallMethod<FString>(GetEmailMethod);
}

void FAndroidSentryUserFeedback::SetComment(const FString& comment)
{
	CallMethod<void>(SetCommentMethod, *GetJString(comment));
}

FString FAndroidSentryUserFeedback::GetComment() const
{
	return CallMethod<FString>(GetCommentMethod);
}
