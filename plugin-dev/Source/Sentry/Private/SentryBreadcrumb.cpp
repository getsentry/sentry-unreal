// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryBreadcrumb.h"

#include "HAL/PlatformSentryBreadcrumb.h"

FSentryBreadcrumb::FSentryBreadcrumb() : NativeImpl(CreateSharedSentryBreadcrumb())
{
}

void FSentryBreadcrumb::SetMessage(const FString &Message)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetMessage(Message);
}

FString FSentryBreadcrumb::GetMessage() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->GetMessage();
}

void FSentryBreadcrumb::SetType(const FString& Type)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetType(Type);
}

FString FSentryBreadcrumb::GetType() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->GetType();
}

void FSentryBreadcrumb::SetCategory(const FString& Category)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetCategory(Category);
}

FString FSentryBreadcrumb::GetCategory() const
{
	if(!NativeImpl)
		return FString();

	return NativeImpl->GetCategory();
}

void FSentryBreadcrumb::SetData(const TMap<FString, FString>& Data)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetData(Data);
}

TMap<FString, FString> FSentryBreadcrumb::GetData() const
{
	if(!NativeImpl)
		return TMap<FString, FString>();

	return NativeImpl->GetData();
}
	
void FSentryBreadcrumb::SetLevel(ESentryLevel Level)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetLevel(Level);
}

ESentryLevel FSentryBreadcrumb::GetLevel() const
{
	if(!NativeImpl)
		return ESentryLevel::Debug;

	return NativeImpl->GetLevel();
}

void USentryBreadcrumbLibrary::SetMessage(FSentryBreadcrumb& Breadcrumb, const FString& Message)
{
	Breadcrumb.SetMessage(Message);
}

FString USentryBreadcrumbLibrary::GetMessage(const FSentryBreadcrumb& Breadcrumb)
{
	return Breadcrumb.GetMessage();
}

void USentryBreadcrumbLibrary::SetType(FSentryBreadcrumb& Breadcrumb, const FString& Type)
{
	Breadcrumb.SetType(Type);
}

FString USentryBreadcrumbLibrary::GetType(const FSentryBreadcrumb& Breadcrumb)
{
	return Breadcrumb.GetType();
}

void USentryBreadcrumbLibrary::SetCategory(FSentryBreadcrumb& Breadcrumb, const FString& Category)
{
	Breadcrumb.SetCategory(Category);
}

FString USentryBreadcrumbLibrary::GetCategory(const FSentryBreadcrumb& Breadcrumb)
{
	return Breadcrumb.GetCategory();
}

void USentryBreadcrumbLibrary::SetData(FSentryBreadcrumb& Breadcrumb, const TMap<FString, FString>& Data)
{
	Breadcrumb.SetData(Data);
}

TMap<FString, FString> USentryBreadcrumbLibrary::GetData(const FSentryBreadcrumb& Breadcrumb)
{
	return Breadcrumb.GetData();
}

void USentryBreadcrumbLibrary::SetLevel(FSentryBreadcrumb& Breadcrumb, ESentryLevel Level)
{
	Breadcrumb.SetLevel(Level);
}

ESentryLevel USentryBreadcrumbLibrary::GetLevel(const FSentryBreadcrumb& Breadcrumb)
{
	return Breadcrumb.GetLevel();
}
