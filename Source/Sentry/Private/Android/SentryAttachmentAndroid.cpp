// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachmentAndroid.h"
#include "Infrastructure/SentryMethodCallAndroid.h"
#include "Infrastructure/SentryConvertorsAndroid.h"

#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"

SentryAttachmentAndroid::SentryAttachmentAndroid(const TArray<uint8>& data, const FString& filename, const FString& contentType)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass attachmentClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/Attachment");
	jmethodID attachmentCtor = Env->GetMethodID(attachmentClass, "<init>", "([BLjava/lang/String;Ljava/lang/String;)V");
	jobject attachmentObject= Env->NewObject(attachmentClass, attachmentCtor,
		SentryConvertorsAndroid::ByteArrayToNative(data),
		SentryConvertorsAndroid::StringToNative(filename),
		SentryConvertorsAndroid::StringToNative(contentType));
	AttachmentAndroid = Env->NewGlobalRef(attachmentObject);
}

SentryAttachmentAndroid::SentryAttachmentAndroid(const FString& path, const FString& filename, const FString& contentType)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass attachmentClass = AndroidJavaEnv::FindJavaClassGlobalRef("io/sentry/Attachment");
	jmethodID attachmentCtor = Env->GetMethodID(attachmentClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	jobject attachmentObject= Env->NewObject(attachmentClass, attachmentCtor,
		SentryConvertorsAndroid::StringToNative(path),
		SentryConvertorsAndroid::StringToNative(filename),
		SentryConvertorsAndroid::StringToNative(contentType));
	AttachmentAndroid = Env->NewGlobalRef(attachmentObject);
}

SentryAttachmentAndroid::~SentryAttachmentAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	Env->DeleteGlobalRef(AttachmentAndroid);
}

jobject SentryAttachmentAndroid::GetNativeObject()
{
	return AttachmentAndroid;
}

TArray<uint8> SentryAttachmentAndroid::GetData() const
{
	jbyteArray data = static_cast<jbyteArray>(SentryMethodCallAndroid::CallObjectMethod(AttachmentAndroid, "getBytes", "()[B"));
	return SentryConvertorsAndroid::ByteArrayToUnreal(data);
}

FString SentryAttachmentAndroid::GetPath() const
{
	return SentryMethodCallAndroid::CallStringMethod(AttachmentAndroid, "getPathname", "()Ljava/lang/String;");
}

FString SentryAttachmentAndroid::GetFilename() const
{
	return SentryMethodCallAndroid::CallStringMethod(AttachmentAndroid, "getFilename", "()Ljava/lang/String;");
}

FString SentryAttachmentAndroid::GetContentType() const
{
	return SentryMethodCallAndroid::CallStringMethod(AttachmentAndroid, "getContentType", "()Ljava/lang/String;");
}