// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AppleSentryUserFeedback.h"

#include "AppleSentryId.h"

#include "Convenience/AppleSentryInclude.h"

FAppleSentryUserFeedback::FAppleSentryUserFeedback(TSharedPtr<ISentryId> eventId)
{
	TSharedPtr<FAppleSentryId> idIOS = StaticCastSharedPtr<FAppleSentryId>(eventId);
	SentryId* id = idIOS->GetNativeObject();

	UserFeedbackApple = [[SentryFeedback alloc] initWithMessage:@""
														   name:nil
														  email:nil
														 source:SentryFeedbackSourceCustom
											  associatedEventId:id
													attachments:nil];
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
