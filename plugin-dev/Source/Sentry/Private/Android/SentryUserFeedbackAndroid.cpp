// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedbackAndroid.h"

#include "SentryId.h"
#include "SentryIdAndroid.h"

SentryUserFeedbackAndroid::SentryUserFeedbackAndroid(USentryId* eventId)
	: FJavaClassObject(GetClassName(), "(Lio/sentry/protocol/SentryId;)V",
		StaticCastSharedPtr<SentryIdAndroid>(eventId->GetNativeImpl())->GetJObject())
	, SetNameMethod(GetClassMethod("setName", "(Ljava/lang/String;)V"))
	, GetNameMethod(GetClassMethod("getName", "()Ljava/lang/String;"))
	, SetEmailMethod(GetClassMethod("setEmail", "(Ljava/lang/String;)V"))
	, GetEmailMethod(GetClassMethod("getEmail", "()Ljava/lang/String;"))
	, SetCommentMethod(GetClassMethod("setComments", "(Ljava/lang/String;)V"))
	, GetCommentMethod(GetClassMethod("getComments", "()Ljava/lang/String;"))
{
}

FName SentryUserFeedbackAndroid::GetClassName()
{
	return FName("io/sentry/UserFeedback");
}

void SentryUserFeedbackAndroid::SetName(const FString& name)
{
	CallMethod<void>(SetNameMethod, *GetJString(name));
}

FString SentryUserFeedbackAndroid::GetName() const
{
	return const_cast<SentryUserFeedbackAndroid*>(this)->CallMethod<FString>(GetNameMethod);
}

void SentryUserFeedbackAndroid::SetEmail(const FString& email)
{
	CallMethod<void>(SetEmailMethod, *GetJString(email));
}

FString SentryUserFeedbackAndroid::GetEmail() const
{
	return const_cast<SentryUserFeedbackAndroid*>(this)->CallMethod<FString>(GetEmailMethod);
}

void SentryUserFeedbackAndroid::SetComment(const FString& comment)
{
	CallMethod<void>(SetCommentMethod, *GetJString(comment));
}

FString SentryUserFeedbackAndroid::GetComment() const
{
	return const_cast<SentryUserFeedbackAndroid*>(this)->CallMethod<FString>(GetCommentMethod);
}
