// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryHintAndroid.h"

#include "SentryAttachmentAndroid.h"

#include "SentryAttachment.h"

#include "Infrastructure/SentryJavaClasses.h"

SentryHintAndroid::SentryHintAndroid()
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryHint, "()V")
{
	SetupClassMethods();
}

SentryHintAndroid::SentryHintAndroid(jobject hint)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryHint, hint)
{
	SetupClassMethods();
}

void SentryHintAndroid::SetupClassMethods()
{
	AddAttachmentMethod = GetMethod("addAttachment", "(Lio/sentry/Attachment;)V");
}

void SentryHintAndroid::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<SentryAttachmentAndroid> attachmentAndroid = StaticCastSharedPtr<SentryAttachmentAndroid>(attachment);
	CallMethod<void>(AddAttachmentMethod, attachmentAndroid->GetJObject());
}