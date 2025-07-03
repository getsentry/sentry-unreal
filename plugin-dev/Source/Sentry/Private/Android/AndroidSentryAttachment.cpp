// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryAttachment.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryAttachment::FAndroidSentryAttachment(const TArray<uint8>& data, const FString& filename, const FString& contentType)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Attachment, "([BLjava/lang/String;Ljava/lang/String;)V",
		  FAndroidSentryConverters::ByteArrayToNative(data), *GetJString(filename), *GetJString(contentType))
{
	SetupClassMethods();
}

FAndroidSentryAttachment::FAndroidSentryAttachment(const FString& path, const FString& filename, const FString& contentType)
	: FSentryJavaObjectWrapper(SentryJavaClasses::Attachment, "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
		  *GetJString(path), *GetJString(filename), *GetJString(contentType))
{
	SetupClassMethods();
}

void FAndroidSentryAttachment::SetupClassMethods()
{
	GetDataMethod = GetMethod("getBytes", "()[B");
	GetPathMethod = GetMethod("getPathname", "()Ljava/lang/String;");
	GetFilenameMethod = GetMethod("getFilename", "()Ljava/lang/String;");
	GetContentTypeMethod = GetMethod("getContentType", "()Ljava/lang/String;");
}

TArray<uint8> FAndroidSentryAttachment::GetData() const
{
	auto data = CallObjectMethod<jobject>(GetDataMethod);
	return FAndroidSentryConverters::ByteArrayToUnreal(static_cast<jbyteArray>(*data));
}

FString FAndroidSentryAttachment::GetPath() const
{
	return CallMethod<FString>(GetPathMethod);
}

FString FAndroidSentryAttachment::GetFilename() const
{
	return CallMethod<FString>(GetFilenameMethod);
}

FString FAndroidSentryAttachment::GetContentType() const
{
	return CallMethod<FString>(GetContentTypeMethod);
}