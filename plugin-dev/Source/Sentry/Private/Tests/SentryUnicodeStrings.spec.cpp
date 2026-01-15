// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryBreadcrumb.h"
#include "SentryEvent.h"
#include "SentryTests.h"
#include "SentryUser.h"

#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryBreadcrumb.h"
#include "HAL/PlatformSentryEvent.h"
#include "HAL/PlatformSentryUser.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryUnicodeStringsSpec, "Sentry.SentryUnicodeStrings", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
END_DEFINE_SPEC(SentryUnicodeStringsSpec)

void SentryUnicodeStringsSpec::Define()
{
	Describe("User", [this]()
	{
		It("should preserve non-ASCII characters in all fields", [this]()
		{
			USentryUser* SentryUser = USentryUser::Create(MakeShareable(new FPlatformSentryUser));

			const FString TestUsername = TEXT("用户🎮이름");
			const FString TestEmail = TEXT("テスト@例.日本");
			const FString TestId = TEXT("아이디123中文");

			TMap<FString, FString> TestData;
			TestData.Add(TEXT("키"), TEXT("한국어값"));
			TestData.Add(TEXT("日本語キー"), TEXT("Value値"));
			TestData.Add(TEXT("mixed"), TEXT("Hello世界🌍"));

			SentryUser->SetUsername(TestUsername);
			SentryUser->SetEmail(TestEmail);
			SentryUser->SetId(TestId);
			SentryUser->SetData(TestData);

			TestEqual("Username", SentryUser->GetUsername(), TestUsername);
			TestEqual("Email", SentryUser->GetEmail(), TestEmail);
			TestEqual("Id", SentryUser->GetId(), TestId);

			TMap<FString, FString> ReceivedData = SentryUser->GetData();
			TestEqual("Data entry count", ReceivedData.Num(), 3);

			const FString* UserValue1 = ReceivedData.Find(TEXT("키"));
			const FString* UserValue2 = ReceivedData.Find(TEXT("日本語キー"));
			const FString* UserValue3 = ReceivedData.Find(TEXT("mixed"));

			TestNotNull("Data key 1 exists", UserValue1);
			TestNotNull("Data key 2 exists", UserValue2);
			TestNotNull("Data key 3 exists", UserValue3);

			if (UserValue1) TestEqual("Data with non-ASCII key", *UserValue1, TEXT("한국어값"));
			if (UserValue2) TestEqual("Data with non-ASCII key and value", *UserValue2, TEXT("Value値"));
			if (UserValue3) TestEqual("Data with mixed characters", *UserValue3, TEXT("Hello世界🌍"));
		});
	});

	Describe("Event", [this]()
	{
		It("should preserve non-ASCII characters in message and tags", [this]()
		{
			USentryEvent* SentryEvent = USentryEvent::Create(CreateSharedSentryEvent());

			const FString TestMessage = TEXT("Error: 오류 エラー 错误 🚨");
			const FString TagKey = TEXT("환경設定");
			const FString TagValue = TEXT("生产🔧");

			SentryEvent->SetMessage(TestMessage);
			SentryEvent->SetTag(TagKey, TagValue);

			TestEqual("Message", SentryEvent->GetMessage(), TestMessage);
			TestEqual("Tag", SentryEvent->GetTag(TagKey), TagValue);
		});

		It("should preserve non-ASCII characters in contexts and extras", [this]()
		{
			USentryEvent* SentryEvent = USentryEvent::Create(CreateSharedSentryEvent());

			const FString ContextKey = TEXT("游戏状态");
			TMap<FString, FSentryVariant> TestContext;
			TestContext.Add(TEXT("플레이어"), TEXT("山田🎮"));
			TestContext.Add(TEXT("状态"), TEXT("게임중"));

			const FString ExtraKey = TEXT("詳細情報");
			const FString ExtraValue = TEXT("추가 데이터 📊");

			SentryEvent->SetContext(ContextKey, TestContext);
			SentryEvent->SetExtra(ExtraKey, ExtraValue);

			TMap<FString, FSentryVariant> ReceivedContext = SentryEvent->GetContext(ContextKey);
			TestEqual("Context entry count", ReceivedContext.Num(), 2);

			const FSentryVariant* CtxValue1 = ReceivedContext.Find(TEXT("플레이어"));
			const FSentryVariant* CtxValue2 = ReceivedContext.Find(TEXT("状态"));

			TestNotNull("Context key 1 exists", CtxValue1);
			TestNotNull("Context key 2 exists", CtxValue2);

			if (CtxValue1) TestEqual("Context value 1", CtxValue1->GetValue<FString>(), TEXT("山田🎮"));
			if (CtxValue2) TestEqual("Context value 2", CtxValue2->GetValue<FString>(), TEXT("게임중"));
			TestEqual("Extra", SentryEvent->GetExtra(ExtraKey).GetValue<FString>(), ExtraValue);
		});

		It("should preserve non-ASCII characters in fingerprint", [this]()
		{
			USentryEvent* SentryEvent = USentryEvent::Create(CreateSharedSentryEvent());

			TArray<FString> TestFingerprint = { TEXT("오류유형"), TEXT("エラー"), TEXT("错误🔍") };

			SentryEvent->SetFingerprint(TestFingerprint);
			TArray<FString> ReceivedFingerprint = SentryEvent->GetFingerprint();

			TestEqual("Fingerprint count", ReceivedFingerprint.Num(), 3);
			TestEqual("Fingerprint[0]", ReceivedFingerprint[0], TestFingerprint[0]);
			TestEqual("Fingerprint[1]", ReceivedFingerprint[1], TestFingerprint[1]);
			TestEqual("Fingerprint[2]", ReceivedFingerprint[2], TestFingerprint[2]);
		});
	});

	Describe("Breadcrumb", [this]()
	{
		It("should preserve non-ASCII characters in all fields", [this]()
		{
			USentryBreadcrumb* SentryBreadcrumb = USentryBreadcrumb::Create(CreateSharedSentryBreadcrumb());

			const FString TestMessage = TEXT("クリック했습니다 🖱️");
			const FString TestType = TEXT("用户操作");
			const FString TestCategory = TEXT("ナビ게이션");

			TMap<FString, FSentryVariant> TestData;
			TestData.Add(TEXT("화면"), TEXT("メイン画面"));
			TestData.Add(TEXT("按钮"), TEXT("确认🔘"));

			SentryBreadcrumb->SetMessage(TestMessage);
			SentryBreadcrumb->SetType(TestType);
			SentryBreadcrumb->SetCategory(TestCategory);
			SentryBreadcrumb->SetData(TestData);

			TestEqual("Message", SentryBreadcrumb->GetMessage(), TestMessage);
			TestEqual("Type", SentryBreadcrumb->GetType(), TestType);
			TestEqual("Category", SentryBreadcrumb->GetCategory(), TestCategory);

			TMap<FString, FSentryVariant> ReceivedData = SentryBreadcrumb->GetData();
			TestEqual("Data entry count", ReceivedData.Num(), 2);

			const FSentryVariant* Value1 = ReceivedData.Find(TEXT("화면"));
			const FSentryVariant* Value2 = ReceivedData.Find(TEXT("按钮"));

			TestNotNull("Data key 1 exists", Value1);
			TestNotNull("Data key 2 exists", Value2);

			if (Value1) TestEqual("Data value 1", Value1->GetValue<FString>(), TEXT("メイン画面"));
			if (Value2) TestEqual("Data value 2", Value2->GetValue<FString>(), TEXT("确认🔘"));
		});
	});
}

#endif
