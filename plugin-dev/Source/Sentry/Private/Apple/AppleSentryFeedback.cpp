// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryFeedback.h"

#include "AppleSentryId.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

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

SentryFeedback* FAppleSentryFeedback::CreateSentryFeedback(TSharedPtr<FAppleSentryFeedback> feedback)
{
	SentryId* id = nil;
	if (!feedback->EventId.IsEmpty())
	{
		TSharedPtr<FAppleSentryId> idIOS = MakeShareable(new FAppleSentryId(feedback->EventId));
		id = idIOS->GetNativeObject();
	}

	return [[SENTRY_APPLE_CLASS(SentryFeedback) alloc] initWithMessage:feedback->Message.GetNSString()
																  name:feedback->Name.GetNSString()
																 email:feedback->Email.GetNSString()
																source:SentryFeedbackSourceCustom
													 associatedEventId:id
														   attachments:nil];
}
