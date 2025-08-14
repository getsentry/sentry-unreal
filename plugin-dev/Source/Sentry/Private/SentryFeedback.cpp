// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryFeedback.h"

#include "HAL/PlatformSentryFeedback.h"

void USentryFeedback::Initialize(const FString& Message)
{
	if (ensure(!Message.IsEmpty()))
	{
		NativeImpl = MakeShareable(new FPlatformSentryFeedback(Message));
	}
}

FString USentryFeedback::GetMessage() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetMessage();
}

void USentryFeedback::SetName(const FString& Name)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetName(Name);
}

FString USentryFeedback::GetName() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetName();
}

void USentryFeedback::SetContactEmail(const FString& Email)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetContactEmail(Email);
}

FString USentryFeedback::GetContactEmail() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetContactEmail();
}

void USentryFeedback::SetAssociatedEvent(const FString& EventId)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetAssociatedEvent(EventId);
}

FString USentryFeedback::GetAssociatedEvent() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetAssociatedEvent();
}
