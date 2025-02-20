#pragma once

#include "Apple/AppleSentrySubsystem.h"

class FMacSentrySubsystem : public FAppleSentrySubsystem
{
protected:

};

typedef FMacSentrySubsystem FPlatformSentrySubsystem;
