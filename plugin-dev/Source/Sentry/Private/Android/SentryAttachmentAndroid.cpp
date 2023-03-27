// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachmentAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

SentryAttachmentAndroid::SentryAttachmentAndroid(const TArray<uint8>& data, const FString& filename, const FString& contentType)
	: FSentryJavaObjectWrapper(GetClassName(), "([BLjava/lang/String;Ljava/lang/String;)V",
		SentryConvertorsAndroid::ByteArrayToNative(data), *FJavaClassObject::GetJString(filename), *FJavaClassObject::GetJString(contentType))
{
	SetupClassMethods();
}

SentryAttachmentAndroid::SentryAttachmentAndroid(const FString& path, const FString& filename, const FString& contentType)
	: FSentryJavaObjectWrapper(GetClassName(), "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
		*FJavaClassObject::GetJString(path), *FJavaClassObject::GetJString(filename), *FJavaClassObject::GetJString(contentType))
{
	SetupClassMethods();
}

void SentryAttachmentAndroid::SetupClassMethods()
{
	GetDataMethod = GetMethod("getBytes", "()[B");
	GetPathMethod = GetMethod("getPathname", "()Ljava/lang/String;");
	GetFilenameMethod = GetMethod("getFilename", "()Ljava/lang/String;");
	GetContentTypeMethod = GetMethod("getContentType", "()Ljava/lang/String;");
}

FSentryJavaClass SentryAttachmentAndroid::GetClassName()
{
	return FSentryJavaClass { "io/sentry/Attachment", ESentryJavaClassType::External };
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