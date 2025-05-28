// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryUser.h"
#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryUser.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryUserSpec, "Sentry.SentryUser", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryUser* SentryUser;
END_DEFINE_SPEC(SentryUserSpec)

void SentryUserSpec::Define()
{
	BeforeEach([this]()
	{
		SentryUser = USentryUser::Create(MakeShareable(new FPlatformSentryUser));
	});

	Describe("User params", [this]()
	{
		It("should persist their values", [this]()
		{
			const FString TestUsername = FString(TEXT("SentryUser"));
			const FString TestEmail = FString(TEXT("user-mail@example.org"));
			const FString TestIpAddress = FString(TEXT("127.0.0.1"));
			const FString TestId = FString(TEXT("123"));

			TMap<FString, FString> TestData;
			TestData.Add(TEXT("Key1"), TEXT("Val1"));
			TestData.Add(TEXT("Key2"), TEXT("Val2"));

			SentryUser->SetUsername(TestUsername);
			SentryUser->SetEmail(TestEmail);
			SentryUser->SetIpAddress(TestIpAddress);
			SentryUser->SetId(TestId);
			SentryUser->SetData(TestData);

			TestEqual("Username", SentryUser->GetUsername(), TestUsername);
			TestEqual("Email", SentryUser->GetEmail(), TestEmail);
			TestEqual("Ip Address", SentryUser->GetIpAddress(), TestIpAddress);
			TestEqual("Id", SentryUser->GetId(), TestId);

			TMap<FString, FString> ReceivedData = SentryUser->GetData();
			TestEqual("Data 1", ReceivedData[TEXT("Key1")], TestData[TEXT("Key1")]);
			TestEqual("Data 2", ReceivedData[TEXT("Key2")], TestData[TEXT("Key2")]);
		});
	});

	Describe("User IP address", [this]()
	{
		It("should be empty if not set", [this]()
		{
			TestTrue("Ip Address", SentryUser->GetIpAddress().IsEmpty());
		});

		It("should be empty if initialized with empty string", [this]()
		{
			SentryUser->SetIpAddress(TEXT(""));
			TestTrue("Ip Address", SentryUser->GetIpAddress().IsEmpty());
		});
	});
}

#endif
