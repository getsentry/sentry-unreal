// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryUserFeedback.h"
#include "SentryId.h"

#include "Interface/SentryUserFeedbackInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryUserFeedbackAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentryUserFeedbackApple.h"
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentryUserFeedbackDesktop.h"
#endif

void USentryUserFeedback::Initialize(USentryId* EventId)
{
#if PLATFORM_ANDROID
	UserFeedbackNativeImpl = MakeShareable(new SentryUserFeedbackAndroid(EventId->GetNativeImpl()));
#elif PLATFORM_IOS || PLATFORM_MAC
	UserFeedbackNativeImpl = MakeShareable(new SentryUserFeedbackApple(EventId->GetNativeImpl()));
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
	UserFeedbackNativeImpl = MakeShareable(new SentryUserFeedbackDesktop(EventId->GetNativeImpl()));
#endif
}

void USentryUserFeedback::SetName(const FString& Name)
{
	if (!UserFeedbackNativeImpl)
		return;

	UserFeedbackNativeImpl->SetName(Name);
}

FString USentryUserFeedback::GetName() const
{
	if(!UserFeedbackNativeImpl)
		return FString();

	return UserFeedbackNativeImpl->GetName();
}

void USentryUserFeedback::SetEmail(const FString& Email)
{
	if (!UserFeedbackNativeImpl)
		return;

	UserFeedbackNativeImpl->SetEmail(Email);
}

FString USentryUserFeedback::GetEmail() const
{
	if(!UserFeedbackNativeImpl)
		return FString();

	return UserFeedbackNativeImpl->GetEmail();
}

void USentryUserFeedback::SetComment(const FString& Comments)
{
	if (!UserFeedbackNativeImpl)
		return;

	UserFeedbackNativeImpl->SetComment(Comments);
}

FString USentryUserFeedback::GetComment() const
{
	if (!UserFeedbackNativeImpl)
		return FString();

	return UserFeedbackNativeImpl->GetComment();
}

void USentryUserFeedback::InitWithNativeImpl(TSharedPtr<ISentryUserFeedback> userFeedbackImpl)
{
	UserFeedbackNativeImpl = userFeedbackImpl;
}

TSharedPtr<ISentryUserFeedback> USentryUserFeedback::GetNativeImpl()
{
	return UserFeedbackNativeImpl;
}