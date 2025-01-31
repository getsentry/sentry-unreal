#pragma once

#if PLATFORM_ANDROID
#include "Android/SentrySubsystemAndroid.h"
#elif PLATFORM_IOS || PLATFORM_MAC
#include "Apple/SentrySubsystemApple.h"
#elif PLATFORM_WINDOWS
#include "Windows/WindowsSentrySubsystem.h"
#elif PLATFORM_LINUX
#include "Linux/LinuxSentrySubsystem.h"
#endif
