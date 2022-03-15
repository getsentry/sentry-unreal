// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScope.h"

#if PLATFORM_ANDROID
#include "Android/SentryScopeAndroid.h"
#endif

#if PLATFORM_IOS
#include "IOS/SentryScopeIOS.h"
#endif

USentryScope::USentryScope()
{
#if PLATFORM_ANDROID
	if (USentryScope::StaticClass()->GetDefaultObject() != this)
	{
		_scopeNativeImplAndroid = MakeShareable(new SentryScopeAndroid());
		_scopeNativeImplAndroid->Init();
	}
#endif
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
#if PLATFORM_ANDROID
	_scopeNativeImplAndroid->SetTagValue(Key, Value);
#endif
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetTagValue(Key, Value);
#endif
}

void USentryScope::RemoveTag(const FString& Key)
{
#if PLATFORM_ANDROID
	_scopeNativeImplAndroid->RemoveTag(Key);
#endif
#if PLATFORM_IOS
	_scopeNativeImplIOS->RemoveTag(Key);
#endif
}

void USentryScope::SetTags(const TMap<FString, FString>& Tags)
{
#if PLATFORM_ANDROID
	_scopeNativeImplAndroid->SetTags(Tags);
#endif
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetTags(Tags);
#endif
}

void USentryScope::SetDist(const FString& Dist)
{
#if PLATFORM_ANDROID
	_scopeNativeImplAndroid->SetDist(Dist);
#endif
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetDist(Dist);
#endif
}

void USentryScope::SetEnvironment(const FString& Environment)
{
#if PLATFORM_ANDROID
	_scopeNativeImplAndroid->SetEnvironment(Environment);
#endif
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetEnvironment(Environment);
#endif
}

void USentryScope::SetFingerprint(const TArray<FString>& Fingerprint)
{
#if PLATFORM_ANDROID
	_scopeNativeImplAndroid->SetFingerprint(Fingerprint);
#endif
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetFingerprint(Fingerprint);
#endif
}

void USentryScope::SetLevel(ESentryLevel Level)
{
#if PLATFORM_ANDROID
	_scopeNativeImplAndroid->SetLevel(Level);
#endif
#if PLATFORM_IOS
	_scopeNativeImplIOS->SetLevel(Level);
#endif
}

void USentryScope::Clear()
{
#if PLATFORM_ANDROID
	_scopeNativeImplAndroid->Clear();
#endif
#if PLATFORM_IOS
	_scopeNativeImplIOS->Clear();
#endif
}

#if PLATFORM_ANDROID
void USentryScope::InitWithNativeImplAndroid(TSharedPtr<SentryScopeAndroid> scopeImpl)
{
	_scopeNativeImplAndroid = scopeImpl;
}

TSharedPtr<SentryScopeAndroid> USentryScope::GetNativeImplAndroid()
{
	return _scopeNativeImplAndroid;
}
#endif

#if PLATFORM_IOS
void USentryScope::InitWithNativeImplIOS(TSharedPtr<SentryScopeIOS> scopeImpl)
{
	_scopeNativeImplIOS = scopeImpl;
}

TSharedPtr<SentryScopeIOS> USentryScope::GetNativeImplIOS()
{
	return _scopeNativeImplIOS;
}
#endif
