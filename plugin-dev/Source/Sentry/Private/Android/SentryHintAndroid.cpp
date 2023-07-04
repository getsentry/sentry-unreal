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

void SentryHintAndroid::AddAttachment(USentryAttachment* attachment)
{
	TSharedPtr<SentryAttachmentAndroid> attachmentAndroid = StaticCastSharedPtr<SentryAttachmentAndroid>(attachment->GetNativeImpl());
	CallMethod<void>(AddAttachmentMethod, attachmentAndroid->GetJObject());
}