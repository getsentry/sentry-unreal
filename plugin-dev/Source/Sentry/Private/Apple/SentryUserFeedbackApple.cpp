// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedbackApple.h"

#include "SentryIdApple.h"

#include "Convenience/SentryInclude.h"
#include "Convenience/SentryMacro.h"

SentryUserFeedbackApple::SentryUserFeedbackApple(TSharedPtr<ISentryId> eventId)
{
	TSharedPtr<SentryIdApple> idIOS = StaticCastSharedPtr<SentryIdApple>(eventId);
	SentryId* id = idIOS->GetNativeObject();

	// `SentryFeedback` is defined in Swift so its name that can be recognized by UE should be taken from "Sentry-Swift.h" to successfully load class on Mac
#if PLATFORM_MAC
	//UserFeedbackApple = [[SENTRY_APPLE_CLASS(_TtC6Sentry14SentryFeedback) alloc] initWithMessage:@""
	//	name:nil email:nil source:SentryFeedbackSourceCustom associatedEventId:id attachments:nil];
	UserFeedbackApple = [[SENTRY_APPLE_CLASS(SentryFeedback) alloc] initWithMessage:@""
			name:nil email:nil source:SentryFeedbackSourceCustom associatedEventId:id attachments:nil];

#elif PLATFORM_IOS
	UserFeedbackApple = [[SENTRY_APPLE_CLASS(SentryFeedback) alloc] initWithMessage:@""
		name:nil email:nil source:SentryFeedbackSourceCustom associatedEventId:id attachments:nil];
#endif
}

SentryUserFeedbackApple::~SentryUserFeedbackApple()
{
	// Put custom destructor logic here if needed
}

SentryFeedback* SentryUserFeedbackApple::GetNativeObject()
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
	UserFeedbackApple.message = comment.GetNSString();
}

FString SentryUserFeedbackApple::GetComment() const
{
	return FString(UserFeedbackApple.message);
}
