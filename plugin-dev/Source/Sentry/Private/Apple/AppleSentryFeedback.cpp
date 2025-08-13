// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryFeedback.h"

#include "AppleSentryId.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryFeedback::FAppleSentryFeedback(const FString& message)
	: Message(message)
{
	UserFeedbackApple = [[SENTRY_APPLE_CLASS(SentryFeedback) alloc] initWithMessage:message.GetNSString()
																			   name:nil
																			  email:nil
																			 source:SentryFeedbackSourceCustom
																  associatedEventId:nil
																		attachments:nil];
}

FAppleSentryFeedback::~FAppleSentryFeedback()
{
	// Put custom destructor logic here if needed
}

void FAppleSentryFeedback::SetNativeObject(SentryFeedback* feedback)
{
	UserFeedbackApple = feedback;
}

SentryFeedback* FAppleSentryFeedback::GetNativeObject()
{
	return UserFeedbackApple;
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
	TSharedPtr<FAppleSentryId> idIOS = MakeShareable(new FAppleSentryId(feedback->EventId));
	SentryId* id = idIOS->GetNativeObject();

	return [[SENTRY_APPLE_CLASS(SentryFeedback) alloc] initWithMessage:feedback->Message.GetNSString()
																  name:feedback->Name.GetNSString()
																 email:feedback->Email.GetNSString()
																source:SentryFeedbackSourceCustom
													 associatedEventId:id
														   attachments:nil];
}
