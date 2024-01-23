// Copyright (c) 2024 Sentry. All Rights Reserved.

#include "SentrySamplingContext.h"

#include "Interface/SentrySamplingContextInterface.h"

#if PLATFORM_ANDROID
#include "Android/SentrySamplingContextAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentrySamplingContextApple.h"
#endif

USentrySamplingContext::USentrySamplingContext()
{
}

void USentrySamplingContext::InitWithNativeImpl(TSharedPtr<ISentrySamplingContext> samplingContextImpl)
{
	SentrySamplingContextNativeImpl = samplingContextImpl;
}

TSharedPtr<ISentrySamplingContext> USentrySamplingContext::GetNativeImpl()
{
	return SentrySamplingContextNativeImpl;
}
