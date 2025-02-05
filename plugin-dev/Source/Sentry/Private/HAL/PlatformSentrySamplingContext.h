#pragma once

#include "Null/NullSentrySamplingContext.h"

static TSharedPtr<ISentrySamplingContext> CreateSharedSentrySamplingContext()
{
    // Sampling context is supposed to be created internally by the SDK using the platform-specific implementations.
    // Currently, it doesn't provide default constructor for Apple/Android thus we can only return Null-version here.
    return MakeShareable(new FNullSentrySamplingContext);
}
