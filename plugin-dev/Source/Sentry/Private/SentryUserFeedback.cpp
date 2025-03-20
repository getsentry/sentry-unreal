// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedback.h"
#include "SentryId.h"

#include "HAL/PlatformSentryUserFeedback.h"

void USentryUserFeedback::Initialize(const FSentryId& EventId)
{
	if (ensure(EventId.GetNativeObject()))
	{
		NativeImpl = CreateSharedSentryUserFeedback(EventId.GetNativeObject());
	}
}

void USentryUserFeedback::SetName(const FString& Name)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetName(Name);
}

FString USentryUserFeedback::GetName() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->GetName();
}

void USentryUserFeedback::SetEmail(const FString& Email)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetEmail(Email);
}

FString USentryUserFeedback::GetEmail() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->GetEmail();
}

void USentryUserFeedback::SetComment(const FString& Comments)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetComment(Comments);
}

FString USentryUserFeedback::GetComment() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetComment();
}
