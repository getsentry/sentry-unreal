#pragma once

#if PLATFORM_ANDROID
#include "Android/SentrySamplingContextAndroid.h"
#elif PLATFORM_APPLE
#include "Apple/SentrySamplingContextApple.h"
#else
#include "Null/NullSentrySamplingContext.h"
#endif

static TSharedPtr<ISentrySamplingContext> CreateSharedSentrySamplingContext()
{
#if PLATFORM_ANDROID
    return MakeShareable(new SentrySamplingContextAndroid);
#elif PLATFORM_APPLE
    return MakeShareable(new SentrySamplingContextApple);
#else
    return MakeShareable(new FNullSentrySamplingContext);
#endif
}
