// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryScope.h"
#include "SentryAttachment.h"
#include "SentryBreadcrumb.h"

#include "HAL/PlatformSentryScope.h"

void USentryScope::Initialize()
{
	NativeImpl = CreateSharedSentryScope();
}

void USentryScope::AddBreadcrumb(USentryBreadcrumb* Breadcrumb)
{
	if (!NativeImpl)
		return;

	NativeImpl->AddBreadcrumb(Breadcrumb->GetNativeObject());
}

void USentryScope::ClearBreadcrumbs()
{
	if (!NativeImpl)
		return;

	NativeImpl->ClearBreadcrumbs();
}

void USentryScope::AddAttachment(USentryAttachment* Attachment)
{
	if (!NativeImpl)
		return;

	NativeImpl->AddAttachment(Attachment->GetNativeObject());
}

void USentryScope::ClearAttachments()
{
	if (!NativeImpl)
		return;

	NativeImpl->ClearAttachments();
}

void USentryScope::SetTagValue(const FString& Key, const FString& Value)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetTagValue(Key, Value);
}

FString USentryScope::GetTagValue(const FString& Key) const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetTagValue(Key);
}

void USentryScope::RemoveTag(const FString& Key)
{
	if (!NativeImpl)
		return;

	NativeImpl->RemoveTag(Key);
}

void USentryScope::SetTags(const TMap<FString, FString>& Tags)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetTags(Tags);
}

TMap<FString, FString> USentryScope::GetTags() const
{
	if (!NativeImpl)
		return TMap<FString, FString>();

	return NativeImpl->GetTags();
}

void USentryScope::SetFingerprint(const TArray<FString>& Fingerprint)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetFingerprint(Fingerprint);
}

TArray<FString> USentryScope::GetFingerprint() const
{
	if (!NativeImpl)
		return TArray<FString>();

	return NativeImpl->GetFingerprint();
}

void USentryScope::SetLevel(ESentryLevel Level)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetLevel(Level);
}

ESentryLevel USentryScope::GetLevel() const
{
	if (!NativeImpl)
		return ESentryLevel::Debug;

	return NativeImpl->GetLevel();
}

void USentryScope::SetContext(const FString& Key, const TMap<FString, FString>& Values)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetContext(Key, Values);
}

void USentryScope::RemoveContext(const FString& Key)
{
	if (!NativeImpl)
		return;

	NativeImpl->RemoveContext(Key);
}

void USentryScope::SetExtraValue(const FString& Key, const FString& Value)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetExtraValue(Key, Value);
}

FString USentryScope::GetExtraValue(const FString& Key) const
{
	if (!NativeImpl)
		return FString();

	return NativeImpl->GetExtraValue(Key);
}

void USentryScope::RemoveExtra(const FString& Key)
{
	if (!NativeImpl)
		return;

	NativeImpl->RemoveExtra(Key);
}

void USentryScope::SetExtras(const TMap<FString, FString>& Extras)
{
	if (!NativeImpl)
		return;

	NativeImpl->SetExtras(Extras);
}

TMap<FString, FString> USentryScope::GetExtras() const
{
	if (!NativeImpl)
		return TMap<FString, FString>();

	return NativeImpl->GetExtras();
}

void USentryScope::Clear()
{
	if (!NativeImpl)
		return;

	NativeImpl->Clear();
}
