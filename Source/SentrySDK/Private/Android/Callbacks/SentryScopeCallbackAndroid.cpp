// Copyright (c) 2022 Sentry. All Rights Reserved.

#include "SentryScopeCallbackAndroid.h"

USentryScopeCallbackAndroid::USentryScopeCallbackAndroid()
{
	if (USentryScope::StaticClass()->GetDefaultObject() != this)
	{
		AddToRoot();
	}
}

void USentryScopeCallbackAndroid::BindDelegate(const FConfigureScopeDelegate& OnConfigure)
{
	OnConfigureDelegate = OnConfigure;
}

void USentryScopeCallbackAndroid::ExecuteDelegate(USentryScope* Scope)
{
	OnConfigureDelegate.ExecuteIfBound(Scope);
	RemoveFromRoot();
}
