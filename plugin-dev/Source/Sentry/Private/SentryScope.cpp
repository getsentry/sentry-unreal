// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScope.h"
#include "SentryBreadcrumb.h"
#include "SentryAttachment.h"

#include "Interface/SentryScopeInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentryScopeAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentryScopeApple.h"
#elif PLATFORM_WINDOWS || PLATFORM_LINUX
#include "Desktop/SentryScopeDesktop.h"
#endif

USentryScope::USentryScope()
{
	if (USentryScope::StaticClass()->GetDefaultObject() != this)
	{
#if PLATFORM_ANDROID
		ScopeNativeImpl = MakeShareable(new SentryScopeAndroid());
#elif PLATFORM_IOS || PLATFORM_MAC
		ScopeNativeImpl = MakeShareable(new SentryScopeApple());
#elif (PLATFORM_WINDOWS || PLATFORM_LINUX) && USE_SENTRY_NATIVE
		ScopeNativeImpl = MakeShareable(new SentryScopeDesktop());
#endif
	}
}

void USentryScope::AddBreadcrumb(USentryBreadcrumb* Breadcrumb)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->AddBreadcrumb(Breadcrumb->GetNativeImpl());
}

void USentryScope::ClearBreadcrumbs()
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->ClearBreadcrumbs();
}

void USentryScope::AddAttachment(USentryAttachment* Attachment)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->AddAttachment(Attachment->GetNativeImpl());
}

void USentryScope::ClearAttachments()
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->ClearAttachments();
}

void USentryScope::SetTagValue(const FString& Key, const FString& Value)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->SetTagValue(Key, Value);
}

FString USentryScope::GetTagValue(const FString& Key) const
{
	if (!ScopeNativeImpl)
		return FString();

	return ScopeNativeImpl->GetTagValue(Key);
}

void USentryScope::RemoveTag(const FString& Key)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->RemoveTag(Key);
}

void USentryScope::SetTags(const TMap<FString, FString>& Tags)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->SetTags(Tags);
}

TMap<FString, FString> USentryScope::GetTags() const
{
	if (!ScopeNativeImpl)
		return TMap<FString, FString>();

	return ScopeNativeImpl->GetTags();
}

void USentryScope::SetDist(const FString& Dist)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->SetDist(Dist);
}

FString USentryScope::GetDist() const
{
	if (!ScopeNativeImpl)
		return FString();

	return ScopeNativeImpl->GetDist();
}

void USentryScope::SetEnvironment(const FString& Environment)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->SetEnvironment(Environment);
}

FString USentryScope::GetEnvironment() const
{
	if (!ScopeNativeImpl)
		return FString();

	return ScopeNativeImpl->GetEnvironment();
}

void USentryScope::SetFingerprint(const TArray<FString>& Fingerprint)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->SetFingerprint(Fingerprint);
}

TArray<FString> USentryScope::GetFingerprint() const
{
	if (!ScopeNativeImpl)
		return TArray<FString>();

	return ScopeNativeImpl->GetFingerprint();
}

void USentryScope::SetLevel(ESentryLevel Level)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->SetLevel(Level);
}

ESentryLevel USentryScope::GetLevel() const
{
	if(!ScopeNativeImpl)
		return ESentryLevel::Debug;

	return ScopeNativeImpl->GetLevel();
}

void USentryScope::SetContext(const FString& Key, const TMap<FString, FString>& Values)
{
	if(!ScopeNativeImpl)
		return;

	ScopeNativeImpl->SetContext(Key, Values);
}

void USentryScope::RemoveContext(const FString& Key)
{
	if(!ScopeNativeImpl)
		return;

	ScopeNativeImpl->RemoveContext(Key);
}

void USentryScope::SetExtraValue(const FString& Key, const FString& Value)
{
	if(!ScopeNativeImpl)
		return;

	ScopeNativeImpl->SetExtraValue(Key, Value);
}

FString USentryScope::GetExtraValue(const FString& Key) const
{
	if(!ScopeNativeImpl)
		return FString();

	return ScopeNativeImpl->GetExtraValue(Key);
}

void USentryScope::RemoveExtra(const FString& Key)
{
	if(!ScopeNativeImpl)
		return;

	ScopeNativeImpl->RemoveExtra(Key);
}

void USentryScope::SetExtras(const TMap<FString, FString>& Extras)
{
	if(!ScopeNativeImpl)
		return;

	ScopeNativeImpl->SetExtras(Extras);
}

TMap<FString, FString> USentryScope::GetExtras() const
{
	if (!ScopeNativeImpl)
		return TMap<FString, FString>();

	return ScopeNativeImpl->GetExtras();
}

void USentryScope::Clear()
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl->Clear();
}

void USentryScope::InitWithNativeImpl(TSharedPtr<ISentryScope> scopeImpl)
{
	if (!ScopeNativeImpl)
		return;

	ScopeNativeImpl = scopeImpl;
}

TSharedPtr<ISentryScope> USentryScope::GetNativeImpl()
{
	return ScopeNativeImpl;
}
