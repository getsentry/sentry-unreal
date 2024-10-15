// Copyright (c) 2023 Sentry. All Rights Reserved.

#include "SentryHint.h"

#include "SentryAttachment.h"
#include "Interface/SentryHintInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryHintAndroid.h"
#endif

USentryHint::USentryHint()
{
	if (USentryHint::StaticClass()->GetDefaultObject() != this)
	{
#if PLATFORM_ANDROID
		SentryHintNativeImpl = MakeShareable(new SentryHintAndroid());
#endif
	}
}

void USentryHint::AddAttachment(USentryAttachment* Attachment)
{
	if(!SentryHintNativeImpl)
		return;

	SentryHintNativeImpl->AddAttachment(Attachment->GetNativeImpl());
}

void USentryHint::InitWithNativeImpl(TSharedPtr<ISentryHint> hintImpl)
{
	SentryHintNativeImpl = hintImpl;
}

TSharedPtr<ISentryHint> USentryHint::GetNativeImpl()
{
	return SentryHintNativeImpl;
}
