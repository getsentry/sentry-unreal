// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryUserFeedback.h"

#include "AppleSentryId.h"

#include "Convenience/AppleSentryInclude.h"
#include "Convenience/AppleSentryMacro.h"

FAppleSentryUserFeedback::FAppleSentryUserFeedback(TSharedPtr<ISentryId> eventId)
{
	TSharedPtr<FAppleSentryId> idIOS = StaticCastSharedPtr<FAppleSentryId>(eventId);
	SentryId* id = idIOS->GetNativeObject();

#if PLATFORM_MAC
	UserFeedbackApple = [[SENTRY_APPLE_CLASS(_TtC6Sentry12SentryFeedback) alloc] initWithMessage:@""
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

SentryFeedback* FAppleSentryUserFeedback::GetNativeObject()
{
	return UserFeedbackApple;
}

void FAppleSentryUserFeedback::SetName(const FString& name)
{
	UserFeedbackApple.name = name.GetNSString();
}

FString FAppleSentryUserFeedback::GetName() const
{
	return FString(UserFeedbackApple.name);
}

void FAppleSentryUserFeedback::SetEmail(const FString& email)
{
	UserFeedbackApple.email = email.GetNSString();
}

FString FAppleSentryUserFeedback::GetEmail() const
{
	return FString(UserFeedbackApple.email);
}

void FAppleSentryUserFeedback::SetComment(const FString& comment)
{
	UserFeedbackApple.message = comment.GetNSString();
}

FString FAppleSentryUserFeedback::GetComment() const
{
	return FString(UserFeedbackApple.message);
}
