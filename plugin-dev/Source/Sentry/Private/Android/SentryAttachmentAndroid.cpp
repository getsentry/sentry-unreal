// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryAttachmentAndroid.h"

#include "Infrastructure/SentryConvertorsAndroid.h"
#include "Infrastructure/SentryJavaClasses.h"

SentryAttachmentAndroid::SentryAttachmentAndroid(const TArray<uint8>& data, const FString& filename, const FString& contentType)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Attachment, "([BLjava/lang/String;Ljava/lang/String;)V",
		SentryConvertorsAndroid::ByteArrayToNative(data), *GetJString(filename), *GetJString(contentType))
{
	SetupClassMethods();
}

SentryAttachmentAndroid::SentryAttachmentAndroid(const FString& path, const FString& filename, const FString& contentType)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Attachment, "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
		*GetJString(path), *GetJString(filename), *GetJString(contentType))
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

TArray<uint8> SentryAttachmentAndroid::GetData() const
{
	auto data = CallObjectMethod<jobject>(GetDataMethod);
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