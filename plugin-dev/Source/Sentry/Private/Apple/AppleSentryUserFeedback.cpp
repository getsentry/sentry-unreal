// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryUserFeedback.h"

#include "AppleSentryId.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryUserFeedback::FAppleSentryUserFeedback(TSharedPtr<ISentryId> eventId)
	: EventId(eventId)
{
	TSharedPtr<FAppleSentryId> idIOS = StaticCastSharedPtr<FAppleSentryId>(eventId);
	SentryId* id = idIOS->GetNativeObject();

#if PLATFORM_MAC
	UserFeedbackApple = [[SENTRY_APPLE_CLASS(_TtC6Sentry14SentryFeedback) alloc] initWithMessage:@""
																							name:nil
																						   email:nil
																						  source:SentryFeedbackSourceCustom
																			   associatedEventId:id
																					 attachments:nil];
#elif PLATFORM_IOS
	UserFeedbackApple = [[SENTRY_APPLE_CLASS(SentryFeedback) alloc] initWithMessage:@""
																			   name:nil
																			  email:nil
																			 source:SentryFeedbackSourceCustom
																  associatedEventId:id
																		attachments:nil];
#endif
}

FAppleSentryUserFeedback::~FAppleSentryUserFeedback()
{
	// Put custom destructor logic here if needed
}

void FAppleSentryUserFeedback::SetNativeObject(SentryFeedback* feedback)
{
	UserFeedbackApple = feedback;
}

SentryFeedback* FAppleSentryUserFeedback::GetNativeObject()
{
	return UserFeedbackApple;
}

void FAppleSentryUserFeedback::SetName(const FString& name)
{
	Name = name;
}

FString FAppleSentryUserFeedback::GetName() const
{
	return Name;
}

void FAppleSentryUserFeedback::SetEmail(const FString& email)
{
	Email = email;
}

FString FAppleSentryUserFeedback::GetEmail() const
{
	return Email;
}

void FAppleSentryUserFeedback::SetComment(const FString& comment)
{
	Comment = comment;
}

FString FAppleSentryUserFeedback::GetComment() const
{
	return Comment;
}

SentryFeedback* FAppleSentryUserFeedback::CreateSentryFeedback(TSharedPtr<FAppleSentryUserFeedback> feedback)
{
	TSharedPtr<FAppleSentryId> idIOS = StaticCastSharedPtr<FAppleSentryId>(feedback->EventId);
	SentryId* id = idIOS->GetNativeObject();

#if PLATFORM_MAC
	return [[SENTRY_APPLE_CLASS(_TtC6Sentry14SentryFeedback) alloc] initWithMessage:feedback->Comment.GetNSString()
																			   name:feedback->Name.GetNSString()
																			  email:feedback->Email.GetNSString()
																			 source:SentryFeedbackSourceCustom
																  associatedEventId:id
																		attachments:nil];
#elif PLATFORM_IOS
	return [[SENTRY_APPLE_CLASS(SentryFeedback) alloc] initWithMessage:feedback->Comment.GetNSString()
																  name:feedback->Name.GetNSString()
																 email:feedback->Email.GetNSString()
																source:SentryFeedbackSourceCustom
													 associatedEventId:id
														   attachments:nil];
#endif
}
