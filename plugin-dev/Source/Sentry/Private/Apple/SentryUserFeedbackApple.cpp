// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedbackApple.h"

#include "SentryIdApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryUserFeedbackApple::SentryUserFeedbackApple(TSharedPtr<ISentryId> eventId)
{
	TSharedPtr<SentryIdApple> idIOS = StaticCastSharedPtr<SentryIdApple>(eventId);
	SentryId* id = idIOS->GetNativeObject();
	UserFeedbackApple = [[SENTRY_APPLE_CLASS(SentryUserFeedback) alloc] initWithEventId:id];
}

SentryUserFeedbackApple::~SentryUserFeedbackApple()
{
	// Put custom destructor logic here if needed
}

SentryUserFeedback* SentryUserFeedbackApple::GetNativeObject()
{
	return UserFeedbackApple;
}

void SentryUserFeedbackApple::SetName(const FString& name)
{
	UserFeedbackApple.name = name.GetNSString();
}

FString SentryUserFeedbackApple::GetName() const
{
	return FString(UserFeedbackApple.name);
}

void SentryUserFeedbackApple::SetEmail(const FString& email)
{
	UserFeedbackApple.email = email.GetNSString();
}

FString SentryUserFeedbackApple::GetEmail() const
{
	return FString(UserFeedbackApple.email);
}

void SentryUserFeedbackApple::SetComment(const FString& comment)
{
	UserFeedbackApple.comments = comment.GetNSString();
}

FString SentryUserFeedbackApple::GetComment() const
{
	return FString(UserFeedbackApple.comments);
}
