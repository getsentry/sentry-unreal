// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryEditorLibrary.h"

#include "Misc/AutomationTest.h"
#include "Misc/EngineVersionComparison.h"

#if WITH_AUTOMATION_TESTS

#if UE_VERSION_OLDER_THAN(5, 5, 0)
static constexpr EAutomationTestFlags::Type SentryEditorApplicationContextMask = EAutomationTestFlags::ApplicationContextMask;
#else
static constexpr EAutomationTestFlags SentryEditorApplicationContextMask = EAutomationTestFlags_ApplicationContextMask;
#endif

BEGIN_DEFINE_SPEC(SentryEditorLibrarySpec, "Sentry.SentryEditorLibrary", EAutomationTestFlags::ProductFilter | SentryEditorApplicationContextMask)
END_DEFINE_SPEC(SentryEditorLibrarySpec)

void SentryEditorLibrarySpec::Define()
{
	Describe("ResolveSettingName", [this]()
	{
		It("should accept the property name as declared in C++", [this]()
		{
			TestEqual("Resolved name", USentryEditorLibrary::ResolveSettingName(TEXT("EnableTracing")), FName(TEXT("EnableTracing")));
			TestEqual("Resolved name", USentryEditorLibrary::ResolveSettingName(TEXT("bRequireUserConsent")), FName(TEXT("bRequireUserConsent")));
		});

		It("should accept the name exposed by the Python API", [this]()
		{
			TestEqual("Resolved name", USentryEditorLibrary::ResolveSettingName(TEXT("enable_tracing")), FName(TEXT("EnableTracing")));
			TestEqual("Resolved name", USentryEditorLibrary::ResolveSettingName(TEXT("traces_sample_rate")), FName(TEXT("TracesSampleRate")));
		});

		It("should map boolean settings whose b prefix Python drops", [this]()
		{
			TestEqual("Resolved name", USentryEditorLibrary::ResolveSettingName(TEXT("require_user_consent")), FName(TEXT("bRequireUserConsent")));
		});

		It("should ignore case", [this]()
		{
			TestEqual("Resolved name", USentryEditorLibrary::ResolveSettingName(TEXT("enabletracing")), FName(TEXT("EnableTracing")));
		});

		It("should return None for unknown settings", [this]()
		{
			TestTrue("Unknown setting", USentryEditorLibrary::ResolveSettingName(TEXT("no_such_setting")).IsNone());
			TestTrue("Empty name", USentryEditorLibrary::ResolveSettingName(NAME_None).IsNone());
		});
	});
}

#endif
