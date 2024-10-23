// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedbackAndroid.h"

#include "SentryIdAndroid.h"

#include "Infrastructure/SentryJavaClasses.h"

SentryUserFeedbackAndroid::SentryUserFeedbackAndroid(TSharedPtr<ISentryId> eventId)
	: FSentryJavaObjectWrapper(SentryJavaClasses::UserFeedback, "(Lio/sentry/protocol/SentryId;)V",
		StaticCastSharedPtr<SentryIdAndroid>(eventId)->GetJObject())
{
	SetupClassMethods();
}

void SentryUserFeedbackAndroid::SetupClassMethods()
{
	SetNameMethod = GetMethod("setName", "(Ljava/lang/String;)V");
	GetNameMethod = GetMethod("getName", "()Ljava/lang/String;");
	SetEmailMethod = GetMethod("setEmail", "(Ljava/lang/String;)V");
	GetEmailMethod = GetMethod("getEmail", "()Ljava/lang/String;");
	SetCommentMethod = GetMethod("setComments", "(Ljava/lang/String;)V");
	GetCommentMethod = GetMethod("getComments", "()Ljava/lang/String;");
}

void SentryUserFeedbackAndroid::SetName(const FString& name)
{
	CallMethod<void>(SetNameMethod, *GetJString(name));
}

FString SentryUserFeedbackAndroid::GetName() const
{
	return CallMethod<FString>(GetNameMethod);
}

void SentryUserFeedbackAndroid::SetEmail(const FString& email)
{
	CallMethod<void>(SetEmailMethod, *GetJString(email));
}

FString SentryUserFeedbackAndroid::GetEmail() const
{
	return CallMethod<FString>(GetEmailMethod);
}

void SentryUserFeedbackAndroid::SetComment(const FString& comment)
{
	CallMethod<void>(SetCommentMethod, *GetJString(comment));
}

FString SentryUserFeedbackAndroid::GetComment() const
{
	return CallMethod<FString>(GetCommentMethod);
}
