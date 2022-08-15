// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedbackIOS.h"

#include "SentryId.h"
#include "SentryIdIOS.h"

#import <Foundation/Foundation.h>
#import <Sentry/Sentry.h>

SentryUserFeedbackIOS::SentryUserFeedbackIOS(USentryId* eventId)
{
	TSharedPtr<SentryIdIOS> idIOS = StaticCastSharedPtr<SentryIdIOS>(eventId->GetNativeImpl());
	SentryId* id = idIOS->GetNativeObject();
	UserFeedbackIOS = [[SentryUserFeedback alloc] initWithEventId:id];
}

SentryUserFeedbackIOS::~SentryUserFeedbackIOS()
{
	// Put custom destructor logic here if needed
}

SentryUserFeedback* SentryUserFeedbackIOS::GetNativeObject()
{
	return UserFeedbackIOS;
}

void SentryUserFeedbackIOS::SetName(const FString& name)
{
	UserFeedbackIOS.name = name.GetNSString();
}

FString SentryUserFeedbackIOS::GetName() const
{
	return FString(UserFeedbackIOS.name);
}

void SentryUserFeedbackIOS::SetEmail(const FString& email)
{
	UserFeedbackIOS.email = email.GetNSString();
}

FString SentryUserFeedbackIOS::GetEmail() const
{
	return FString(UserFeedbackIOS.email);
}

void SentryUserFeedbackIOS::SetComment(const FString& comment)
{
	UserFeedbackIOS.comments = comment.GetNSString();
}

FString SentryUserFeedbackIOS::GetComment() const
{
	return FString(UserFeedbackIOS.comments);
}
