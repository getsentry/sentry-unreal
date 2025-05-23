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
}

void FAndroidSentryHint::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	TSharedPtr<FAndroidSentryAttachment> attachmentAndroid = StaticCastSharedPtr<FAndroidSentryAttachment>(attachment);
	CallMethod<void>(AddAttachmentMethod, attachmentAndroid->GetJObject());
}