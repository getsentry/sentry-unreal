#pragma once

#include "Apple/AppleSentrySubsystem.h"

class FIOSSentrySubsystem : public FAppleSentrySubsystem
{
protected:
    virtual int32 GetAssertionFramesToSkip() const override { return 5; }
};

typedef FIOSSentrySubsystem FPlatformSentrySubsystem;
