// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#include "Utils/SentryLogUtils.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryCallstackFilterSpec, "Sentry.SentryCallstackFilter", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
END_DEFINE_SPEC(SentryCallstackFilterSpec)

void SentryCallstackFilterSpec::Define()
{
	Describe("Callstack line detection", [this]()
	{
		It("should match prefixed engine frame lines", [this]()
		{
			TestTrue("Prefixed", SentryLogUtils::IsCallstackLine(TEXT("[Callstack] 0x00007ff6a1b2c3d4 MyGame.exe!FDebug::CheckVerifyFailedImpl() [AssertionMacros.cpp:123]")));
		});

		It("should match Apple backtrace frame lines", [this]()
		{
			TestTrue("Index 5", SentryLogUtils::IsCallstackLine(TEXT("5   SentryPlayground                    0x00000001052042b8 FDebug::CheckVerifyFailedImpl2(char const*, char const*, int, char16_t const*, ...) + 248")));
			TestTrue("Index 12", SentryLogUtils::IsCallstackLine(TEXT("12  libsystem_pthread.dylib             0x00000001e3b8a06c _pthread_start + 136")));
			TestTrue("Index 123", SentryLogUtils::IsCallstackLine(TEXT("123 UIKitCore 0x1a2b3c start + 4")));
		});

		It("should not match regular log messages", [this]()
		{
			TestFalse("Empty", SentryLogUtils::IsCallstackLine(TEXT("")));
			TestFalse("Plain", SentryLogUtils::IsCallstackLine(TEXT("=== Critical error: ===")));
			TestFalse("Assertion", SentryLogUtils::IsCallstackLine(TEXT("Assertion failed: false [File:SentryPlaygroundUtils.cpp] [Line: 42]")));
			TestFalse("Number then words", SentryLogUtils::IsCallstackLine(TEXT("3 players joined the session")));
			TestFalse("Number then text with hex", SentryLogUtils::IsCallstackLine(TEXT("42 objects at address 0xZZ")));
			TestFalse("Number only", SentryLogUtils::IsCallstackLine(TEXT("100")));
			TestFalse("Two tokens", SentryLogUtils::IsCallstackLine(TEXT("5 SentryPlayground")));
			TestFalse("Leading hex", SentryLogUtils::IsCallstackLine(TEXT("0x00007ff6a1b2c3d4 not prefixed")));
		});
	});
}

#endif
