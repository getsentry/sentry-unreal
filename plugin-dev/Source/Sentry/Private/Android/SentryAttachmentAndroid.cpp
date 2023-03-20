// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachmentAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

SentryAttachmentAndroid::SentryAttachmentAndroid(const TArray<uint8>& data, const FString& filename, const FString& contentType)
	: FSentryJavaClassWrapper(GetClassName(), "([BLjava/lang/String;Ljava/lang/String;)V",
		SentryConvertorsAndroid::ByteArrayToNative(data), *FJavaClassObject::GetJString(filename), *FJavaClassObject::GetJString(contentType))
{
	SetupClassMethods();
}

SentryAttachmentAndroid::SentryAttachmentAndroid(const FString& path, const FString& filename, const FString& contentType)
	: FSentryJavaClassWrapper(GetClassName(), "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
		*FJavaClassObject::GetJString(path), *FJavaClassObject::GetJString(filename), *FJavaClassObject::GetJString(contentType))
{
	SetupClassMethods();
}

void SentryAttachmentAndroid::SetupClassMethods()
{
	GetDataMethod = GetClassMethod("getBytes", "()[B");
	GetPathMethod = GetClassMethod("getPathname", "()Ljava/lang/String;");
	GetFilenameMethod = GetClassMethod("getFilename", "()Ljava/lang/String;");
	GetContentTypeMethod = GetClassMethod("getContentType", "()Ljava/lang/String;");
}

FName SentryAttachmentAndroid::GetClassName()
{
	return FName("io/sentry/Attachment");
}

TArray<uint8> SentryAttachmentAndroid::GetData() const
{
	auto data = NewSentryScopedJavaObject(CallMethod<jobject>(GetDataMethod));
	return SentryConvertorsAndroid::ByteArrayToUnreal(static_cast<jbyteArray>(*data));
}

FString SentryAttachmentAndroid::GetPath() const
{
	return CallMethod<FString>(GetPathMethod);
}

FString SentryAttachmentAndroid::GetFilename() const
{
	return CallMethod<FString>(GetFilenameMethod);
}

FString SentryAttachmentAndroid::GetContentType() const
{
	return CallMethod<FString>(GetContentTypeMethod);
}