// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedbackAndroid.h"

#include "SentryId.h"
#include "SentryIdAndroid.h"

#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

SentryUserFeedbackAndroid::SentryUserFeedbackAndroid(USentryId* eventId)
{
	TSharedPtr<SentryIdAndroid> idAndroid = StaticCastSharedPtr<SentryIdAndroid>(eventId->GetNativeImpl());

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	jclass userFeedbackClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/UserFeedback");
	jmethodID userFeedbackCtor = Env->GetMethodID(userFeedbackClass, "<init>", "(Lio/sentry/protocol/SentryId;)V");
	jobject userFeedbackObject = Env->NewObject(userFeedbackClass, userFeedbackCtor, idAndroid->GetNativeObject());
	UserFeedbackAndroid = Env->NewGlobalRef(userFeedbackObject);
}

SentryUserFeedbackAndroid::~SentryUserFeedbackAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	Env->DeleteGlobalRef(UserFeedbackAndroid);
}

jobject SentryUserFeedbackAndroid::GetNativeObject()
{
	return UserFeedbackAndroid;
}

void SentryUserFeedbackAndroid::SetName(const FString& name)
{
	SentryMethodCallAndroid::CallVoidMethod(UserFeedbackAndroid, "setName", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(name));
}

FString SentryUserFeedbackAndroid::GetName() const
{
	return SentryMethodCallAndroid::CallStringMethod(UserFeedbackAndroid, "getName", "()Ljava/lang/String;");
}

void SentryUserFeedbackAndroid::SetEmail(const FString& email)
{
	SentryMethodCallAndroid::CallVoidMethod(UserFeedbackAndroid, "setEmail", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(email));
}

FString SentryUserFeedbackAndroid::GetEmail() const
{
	return SentryMethodCallAndroid::CallStringMethod(UserFeedbackAndroid, "getEmail", "()Ljava/lang/String;");
}

void SentryUserFeedbackAndroid::SetComment(const FString& comment)
{
	SentryMethodCallAndroid::CallVoidMethod(UserFeedbackAndroid, "setComments", "(Ljava/lang/String;)V",
		SentryConvertorsAndroid::StringToNative(comment));
}

FString SentryUserFeedbackAndroid::GetComment() const
{
	return SentryMethodCallAndroid::CallStringMethod(UserFeedbackAndroid, "getComments", "()Ljava/lang/String;");
}
