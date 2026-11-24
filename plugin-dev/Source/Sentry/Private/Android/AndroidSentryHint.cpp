// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryHint.h"

#include "AndroidSentryAttachment.h"

#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryHint::FAndroidSentryHint()
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryHint, "()V")
{
	SetupClassMethods();
}

FAndroidSentryHint::FAndroidSentryHint(jobject hint)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryHint, hint)
{
	SetupClassMethods();
}

void FAndroidSentryHint::SetupClassMethods()
{
	AddAttachmentMethod = GetMethod("addAttachment", "(Lio/sentry/Attachment;)V");
	GetAttachmentsMethod = GetMethod("getAttachments", "()Ljava/util/List;");
	ReplaceAttachmentsMethod = GetMethod("replaceAttachments", "(Ljava/util/List;)V");
	ClearAttachmentsMethod = GetMethod("clearAttachments", "()V");
}

void FAndroidSentryHint::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FAndroidSentryAttachment> attachmentAndroid = StaticCastSharedPtr<FAndroidSentryAttachment>(attachment);
	CallMethod<void>(AddAttachmentMethod, attachmentAndroid->GetJObject());
}

int32 FAndroidSentryHint::RemoveAttachments(const FString& filenamePattern)
{
	auto attachments = CallObjectMethod<jobject>(GetAttachmentsMethod);
	if (!attachments)
	{
		return 0;
	}

	FSentryJavaObjectWrapper NativeList(SentryJavaClasses::List, *attachments);
	FSentryJavaMethod SizeMethod = NativeList.GetMethod("size", "()I");
	FSentryJavaMethod GetItemMethod = NativeList.GetMethod("get", "(I)Ljava/lang/Object;");
	FSentryJavaMethod RemoveItemMethod = NativeList.GetMethod("remove", "(I)Ljava/lang/Object;");

	int32 removedCount = 0;

	for (int i = NativeList.CallMethod<int>(SizeMethod) - 1; i >= 0; --i)
	{
		auto item = NativeList.CallObjectMethod<jobject>(GetItemMethod, i);

		FSentryJavaObjectWrapper NativeAttachment(SentryJavaClasses::Attachment, *item);
		FSentryJavaMethod GetFilenameMethod = NativeAttachment.GetMethod("getFilename", "()Ljava/lang/String;");

		if (NativeAttachment.CallMethod<FString>(GetFilenameMethod).MatchesWildcard(filenamePattern))
		{
			NativeList.CallObjectMethod<jobject>(RemoveItemMethod, i);
			++removedCount;
		}
	}

	if (removedCount > 0)
	{
		CallMethod<void>(ReplaceAttachmentsMethod, NativeList.GetJObject());
	}

	return removedCount;
}

void FAndroidSentryHint::ClearAttachments()
{
	CallMethod<void>(ClearAttachmentsMethod);
}