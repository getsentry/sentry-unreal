// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedback.h"
#include "SentryId.h"
#include "Interface/SentryUserFeedbackInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryUserFeedbackAndroid.h"
#endif

#if PLATFORM_IOS
#include "IOS/SentryUserFeedbackIOS.h"
#endif

void USentryUserFeedback::Initialize(USentryId* EventId)
{
#if PLATFORM_ANDROID
	UserFeedbackNativeImpl = MakeShareable(new SentryUserFeedbackAndroid(EventId));
#endif
#if PLATFORM_IOS
	UserFeedbackNativeImpl = MakeShareable(new SentryUserFeedbackIOS(EventId));
#endif
}

void USentryUserFeedback::SetName(const FString& Name)
{
	if (!UserFeedbackNativeImpl)
		return;

	UserFeedbackNativeImpl->SetName(Name);
}

void USentryUserFeedback::SetEmail(const FString& Email)
{
	if (!UserFeedbackNativeImpl)
		return;

	UserFeedbackNativeImpl->SetEmail(Email);
}

void USentryUserFeedback::SetComment(const FString& Comment)
{
	if (!UserFeedbackNativeImpl)
		return;

	UserFeedbackNativeImpl->SetComment(Comment);
}

void USentryUserFeedback::InitWithNativeImpl(TSharedPtr<ISentryUserFeedback> userFeedbackImpl)
{
	UserFeedbackNativeImpl = userFeedbackImpl;
}

TSharedPtr<ISentryUserFeedback> USentryUserFeedback::GetNativeImpl()
{
	return UserFeedbackNativeImpl;
}