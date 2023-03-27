// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedbackAndroid.h"

#include "SentryId.h"
#include "SentryIdAndroid.h"

SentryUserFeedbackAndroid::SentryUserFeedbackAndroid(USentryId* eventId)
	: FSentryJavaObjectWrapper(GetClassName(), "(Lio/sentry/protocol/SentryId;)V",
		StaticCastSharedPtr<SentryIdAndroid>(eventId->GetNativeImpl())->GetJObject())
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

FSentryJavaClass SentryUserFeedbackAndroid::GetClassName()
{
	return FSentryJavaClass{ "io/sentry/UserFeedback", ESentryJavaClassType::External };
}

void SentryUserFeedbackAndroid::SetName(const FString& name)
{
	CallMethod<void>(SetNameMethod, *FJavaClassObject::GetJString(name));
}

FString SentryUserFeedbackAndroid::GetName() const
{
	return CallMethod<FString>(GetNameMethod);
}

void SentryUserFeedbackAndroid::SetEmail(const FString& email)
{
	CallMethod<void>(SetEmailMethod, *FJavaClassObject::GetJString(email));
}

FString SentryUserFeedbackAndroid::GetEmail() const
{
	return CallMethod<FString>(GetEmailMethod);
}

void SentryUserFeedbackAndroid::SetComment(const FString& comment)
{
	CallMethod<void>(SetCommentMethod, *FJavaClassObject::GetJString(comment));
}

FString SentryUserFeedbackAndroid::GetComment() const
{
	return CallMethod<FString>(GetCommentMethod);
}
