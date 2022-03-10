// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScope.h"

#if PLATFORM_IOS
#include "IOS/SentryScopeIOS.h"
#endif

USentryScope::USentryScope()
{
#if PLATFORM_IOS
	if (USentryScope::StaticClass()->GetDefaultObject() != this)
	{
		_scopeNativeImplIOS = MakeShareable(new SentryScopeIOS());
		_scopeNativeImplIOS->Init();
	}
#endif
}

void USentryScope::SetTagValue(const FString& Key, const FString& Value)
{
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetTagValue(Key, Value);
#endif
}

void USentryScope::RemoveTag(const FString& Key)
{
#if PLATFORM_IOS
	_scopeNativeImplIOS->RemoveTag(Key);
#endif
}

void USentryScope::SetTags(const TMap<FString, FString>& Tags)
{
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetTags(Tags);
#endif
}

void USentryScope::SetDist(const FString& Dist)
{
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetDist(Dist);
#endif
}

void USentryScope::SetEnvironment(const FString& Environment)
{
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetEnvironment(Environment);
#endif
}

void USentryScope::SetFingerprint(const TArray<FString>& Fingerprint)
{
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetFingerprint(Fingerprint);
#endif
}

void USentryScope::SetLevel(ESentryLevel Level)
{
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetLevel(Level);
#endif
}

void USentryScope::Clear()
{
#if PLATFORM_IOS
	_scopeNativeImplIOS->Clear();
#endif
}

#if PLATFORM_IOS
void USentryScope::InitWithNativeImplIOS(TSharedPtr<SentryScopeIOS> scope)
{
	_scopeNativeImplIOS = scope;
}
#endif
