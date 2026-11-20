// Copyright (c) 2026 Sentry. All Rights Reserved.

#include "SentryTests.h"

#include "OutputDevice/SentryErrorOutputDevice.h"

#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

namespace SentryErrorOutputDeviceTests
{
class FTestParentErrorDevice : public FOutputDeviceError
{
public:
	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
	{
		Messages.Add(V);
	}

	virtual void HandleError() override
	{
		HandleErrorCount++;
	}

	TArray<FString> Messages;
	int32 HandleErrorCount = 0;
};

class FTestErrorOutputDevice : public FSentryErrorOutputDevice
{
public:
	using FSentryErrorOutputDevice::FSentryErrorOutputDevice;

	bool bHasAsserted = true;

protected:
	virtual bool HasAsserted() const override
	{
		return bHasAsserted;
	}
};
} // namespace SentryErrorOutputDeviceTests

BEGIN_DEFINE_SPEC(SentryErrorOutputDeviceSpec, "Sentry.SentryErrorOutputDevice", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	TUniquePtr<SentryErrorOutputDeviceTests::FTestParentErrorDevice> ParentDevice;
	TUniquePtr<SentryErrorOutputDeviceTests::FTestErrorOutputDevice> ErrorDevice;
END_DEFINE_SPEC(SentryErrorOutputDeviceSpec)

void SentryErrorOutputDeviceSpec::Define()
{
	BeforeEach([this]()
	{
		ParentDevice = MakeUnique<SentryErrorOutputDeviceTests::FTestParentErrorDevice>();
		ErrorDevice = MakeUnique<SentryErrorOutputDeviceTests::FTestErrorOutputDevice>(ParentDevice.Get());
	});

	AfterEach([this]()
	{
		ErrorDevice.Reset();
		ParentDevice.Reset();
	});

	Describe("Assert handling", [this]()
	{
		It("should notify about an assert and forward it to the parent device", [this]()
		{
			TArray<FString> AssertMessages;
			ErrorDevice->OnAssert.AddLambda([&AssertMessages](const FString& Message)
			{
				AssertMessages.Add(Message);
			});

			ErrorDevice->Serialize(TEXT("Assertion failed"), ELogVerbosity::Error, NAME_None);

			TestEqual("Assert notifications", AssertMessages.Num(), 1);
			TestEqual("Assert message", AssertMessages[0], TEXT("Assertion failed"));
			TestEqual("Forwarded messages", ParentDevice->Messages.Num(), 1);
		});

		It("should not notify when no assert happened", [this]()
		{
			ErrorDevice->bHasAsserted = false;

			int32 AssertCount = 0;
			ErrorDevice->OnAssert.AddLambda([&AssertCount](const FString& Message)
			{
				AssertCount++;
			});

			ErrorDevice->Serialize(TEXT("Fatal error"), ELogVerbosity::Fatal, NAME_None);

			TestEqual("Assert notifications", AssertCount, 0);
			TestEqual("Forwarded messages", ParentDevice->Messages.Num(), 1);
		});

		It("should notify only once for subsequent asserts", [this]()
		{
			int32 AssertCount = 0;
			ErrorDevice->OnAssert.AddLambda([&AssertCount](const FString& Message)
			{
				AssertCount++;
			});

			ErrorDevice->Serialize(TEXT("First assert"), ELogVerbosity::Error, NAME_None);
			ErrorDevice->Serialize(TEXT("Second assert"), ELogVerbosity::Error, NAME_None);

			TestEqual("Assert notifications", AssertCount, 1);
			TestEqual("Forwarded messages", ParentDevice->Messages.Num(), 2);
		});

		It("should not re-enter the assert handler when it asserts again", [this]()
		{
			const int32 MaxDepth = 3;
			int32 AssertCount = 0;
			ErrorDevice->OnAssert.AddLambda([this, &AssertCount, MaxDepth](const FString& Message)
			{
				if (++AssertCount < MaxDepth)
				{
					ErrorDevice->Serialize(TEXT("Re-entrant assert"), ELogVerbosity::Error, NAME_None);
				}
			});

			ErrorDevice->Serialize(TEXT("Initial assert"), ELogVerbosity::Error, NAME_None);

			TestEqual("Assert notifications", AssertCount, 1);
			TestEqual("Forwarded messages", ParentDevice->Messages.Num(), 2);
			TestEqual("Re-entrant message forwarded first", ParentDevice->Messages[0], TEXT("Re-entrant assert"));
			TestEqual("Initial message forwarded last", ParentDevice->Messages[1], TEXT("Initial assert"));
		});

		It("should forward error handling to the parent device", [this]()
		{
			ErrorDevice->HandleError();

			TestEqual("Parent error handling calls", ParentDevice->HandleErrorCount, 1);
		});
	});
}

#endif
