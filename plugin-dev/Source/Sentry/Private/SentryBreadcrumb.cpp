// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryBreadcrumb.h"

#include "HAL/PlatformSentryBreadcrumb.h"

void USentryBreadcrumb::Initialize()
{
	NativeImpl = CreateSharedSentryBreadcrumb();
}

void USentryBreadcrumb::SetMessage(const FString& Message)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetMessage(Message);
}

FString USentryBreadcrumb::GetMessage() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetMessage();
}

void USentryBreadcrumb::SetType(const FString& Type)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetType(Type);
}

FString USentryBreadcrumb::GetType() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetType();
}

void USentryBreadcrumb::SetCategory(const FString& Category)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetCategory(Category);
}

FString USentryBreadcrumb::GetCategory() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetCategory();
}

void USentryBreadcrumb::SetData(const TMap<FString, FSentryVariant>& Data)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetData(Data);
}

TMap<FString, FSentryVariant> USentryBreadcrumb::GetData() const
{
	if (!NativeImpl)
		return TMap<FString, FSentryVariant>();

	return NativeImpl->GetData();
}

void USentryBreadcrumb::SetLevel(ESentryLevel Level)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetLevel(Level);
}

ESentryLevel USentryBreadcrumb::GetLevel() const
{
	if (!NativeImpl)
		return ESentryLevel::Debug;

	return NativeImpl->GetLevel();
}
