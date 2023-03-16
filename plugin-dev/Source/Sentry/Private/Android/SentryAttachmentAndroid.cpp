// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachmentAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryScopedJavaObject.h"

SentryAttachmentAndroid::SentryAttachmentAndroid(const TArray<uint8>& data, const FString& filename, const FString& contentType)
	: FJavaClassObject(GetClassName(), "([BLjava/lang/String;Ljava/lang/String;)V",
		SentryConvertorsAndroid::ByteArrayToNative(data), *GetJString(filename), *GetJString(contentType))
	, GetDataMethod(GetClassMethod("getBytes", "()[B"))
	, GetPathMethod(GetClassMethod("getPathname", "()Ljava/lang/String;"))
	, GetFilenameMethod(GetClassMethod("getFilename", "()Ljava/lang/String;"))
	, GetContentTypeMethod(GetClassMethod("getContentType", "()Ljava/lang/String;"))
{
}

SentryAttachmentAndroid::SentryAttachmentAndroid(const FString& path, const FString& filename, const FString& contentType)
	: FJavaClassObject(GetClassName(), "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
		*GetJString(path), *GetJString(filename), *GetJString(contentType))
	, GetDataMethod(GetClassMethod("getBytes", "()[B"))
	, GetPathMethod(GetClassMethod("getPathname", "()Ljava/lang/String;"))
	, GetFilenameMethod(GetClassMethod("getFilename", "()Ljava/lang/String;"))
	, GetContentTypeMethod(GetClassMethod("getContentType", "()Ljava/lang/String;"))
{
}

FName SentryAttachmentAndroid::GetClassName()
{
	return FName("io/sentry/Attachment");
}

TArray<uint8> SentryAttachmentAndroid::GetData() const
{
	jobject data = const_cast<SentryAttachmentAndroid*>(this)->CallMethod<jobject>(GetDataMethod);
	return SentryConvertorsAndroid::ByteArrayToUnreal(data ? *NewSentryScopedJavaObject(static_cast<jbyteArray>(data)) : nullptr);
}

FString SentryAttachmentAndroid::GetPath() const
{
	return const_cast<SentryAttachmentAndroid*>(this)->CallMethod<FString>(GetPathMethod);
}

FString SentryAttachmentAndroid::GetFilename() const
{
	return const_cast<SentryAttachmentAndroid*>(this)->CallMethod<FString>(GetFilenameMethod);
}

FString SentryAttachmentAndroid::GetContentType() const
{
	return const_cast<SentryAttachmentAndroid*>(this)->CallMethod<FString>(GetContentTypeMethod);
}