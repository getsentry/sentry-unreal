// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryFeedback.h"

#if !USE_SENTRY_NATIVE

FAppleSentryFeedback::FAppleSentryFeedback(const FString& message)
	: Message(message)
{
}

FAppleSentryFeedback::~FAppleSentryFeedback()
{
	// Put custom destructor logic here if needed
}

FString FAppleSentryFeedback::GetMessage() const
{
	return Message;
}

void FAppleSentryFeedback::SetName(const FString& name)
{
	Name = name;
}

FString FAppleSentryFeedback::GetName() const
{
	return Name;
}

void FAppleSentryFeedback::SetContactEmail(const FString& email)
{
	Email = email;
}

FString FAppleSentryFeedback::GetContactEmail() const
{
	return Email;
}

void FAppleSentryFeedback::SetAssociatedEvent(const FString& eventId)
{
	EventId = eventId;
}

FString FAppleSentryFeedback::GetAssociatedEvent() const
{
	return EventId;
}

void FAppleSentryFeedback::AddAttachment(TSharedPtr<ISentryAttachment> attachment)
{
	Attachments.Add(attachment);
}

#endif // !USE_SENTRY_NATIVE
