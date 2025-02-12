#pragma once

#include "Apple/AppleSentrySubsystem.h"

class FMacSentrySubsystem : public FAppleSentrySubsystem
{
protected:
    virtual int32 GetAssertionFramesToSkip() const override { return 6; }
};

typedef FMacSentrySubsystem FPlatformSentrySubsystem;
