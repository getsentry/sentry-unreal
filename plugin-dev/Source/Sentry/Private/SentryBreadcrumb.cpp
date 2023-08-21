// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryBreadcrumb.h"

#include "Interface/SentryBreadcrumbInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryBreadcrumbAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentryBreadcrumbApple.h"
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentryBreadcrumbDesktop.h"
#endif

USentryBreadcrumb::USentryBreadcrumb()
{
	if (USentryBreadcrumb::StaticClass()->GetDefaultObject() != this)
	{
#if PLATFORM_ANDROID
		BreadcrumbNativeImpl = MakeShareable(new SentryBreadcrumbAndroid());
#elif PLATFORM_IOS || PLATFORM_MAC
		BreadcrumbNativeImpl = MakeShareable(new SentryBreadcrumbApple());
#elif (PLATFORM_WINDOWS || PLATFORM_LINUX) && USE_SENTRY_NATIVE
		BreadcrumbNativeImpl = MakeShareable(new SentryBreadcrumbDesktop());
#endif
	}
}

void USentryBreadcrumb::SetMessage(const FString& Message)
{
	if (!BreadcrumbNativeImpl)
		return;

	BreadcrumbNativeImpl->SetMessage(Message);
}

FString USentryBreadcrumb::GetMessage() const
{
	if(!BreadcrumbNativeImpl)
		return FString();

	return BreadcrumbNativeImpl->GetMessage();
}

void USentryBreadcrumb::SetType(const FString& Type)
{
	if (!BreadcrumbNativeImpl)
		return;

	BreadcrumbNativeImpl->SetType(Type);
}

FString USentryBreadcrumb::GetType() const
{
	if(!BreadcrumbNativeImpl)
		return FString();

	return BreadcrumbNativeImpl->GetType();
}

void USentryBreadcrumb::SetCategory(const FString& Category)
{
	if (!BreadcrumbNativeImpl)
		return;

	BreadcrumbNativeImpl->SetCategory(Category);
}

FString USentryBreadcrumb::GetCategory() const
{
	if(!BreadcrumbNativeImpl)
		return FString();

	return BreadcrumbNativeImpl->GetCategory();
}

void USentryBreadcrumb::SetData(const TMap<FString, FString>& Data)
{
	if (!BreadcrumbNativeImpl)
		return;

	BreadcrumbNativeImpl->SetData(Data);
}

TMap<FString, FString> USentryBreadcrumb::GetData() const
{
	if(!BreadcrumbNativeImpl)
		return TMap<FString, FString>();

	return BreadcrumbNativeImpl->GetData();
}
	
void USentryBreadcrumb::SetLevel(ESentryLevel Level)
{
	if (!BreadcrumbNativeImpl)
		return;

	BreadcrumbNativeImpl->SetLevel(Level);
}

ESentryLevel USentryBreadcrumb::GetLevel() const
{
	if(!BreadcrumbNativeImpl)
		return ESentryLevel::Debug;

	return BreadcrumbNativeImpl->GetLevel();
}

void USentryBreadcrumb::InitWithNativeImpl(TSharedPtr<ISentryBreadcrumb> breadcrumbImpl)
{
	BreadcrumbNativeImpl = breadcrumbImpl;
}

TSharedPtr<ISentryBreadcrumb> USentryBreadcrumb::GetNativeImpl()
{
	return BreadcrumbNativeImpl;
}
