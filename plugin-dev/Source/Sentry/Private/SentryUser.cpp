// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryUser.h"

#include "HAL/PlatformSentryUser.h"

void USentryUser::Initialize()
{
	NativeImpl = MakeShareable(new FPlatformSentryUser);
}

void USentryUser::SetEmail(const FString& Email)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetEmail(Email);
}

FString USentryUser::GetEmail() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetEmail();
}

void USentryUser::SetId(const FString& Id)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetId(Id);
}

FString USentryUser::GetId() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetId();
}

void USentryUser::SetUsername(const FString& Username)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetUsername(Username);
}

FString USentryUser::GetUsername() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetUsername();
}

void USentryUser::SetIpAddress(const FString& IpAddress)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetIpAddress(IpAddress);
}

FString USentryUser::GetIpAddress() const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetIpAddress();
}

void USentryUser::SetData(const TMap<FString, FString>& Data)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetData(Data);
}

TMap<FString, FString> USentryUser::GetData() const
{
	if (!NativeImpl)
		return TMap<FString, FString>();

	return NativeImpl->GetData();
}
