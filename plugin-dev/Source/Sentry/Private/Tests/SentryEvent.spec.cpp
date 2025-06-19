// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryEvent.h"
#include "SentryTests.h"

#include "Misc/AutomationTest.h"

#include "HAL/PlatformSentryEvent.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryEventSpec, "Sentry.SentryEvent", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
	USentryEvent* SentryEvent;
END_DEFINE_SPEC(SentryEventSpec)

void SentryEventSpec::Define()
{
	BeforeEach([this]()
	{
		SentryEvent = USentryEvent::Create(CreateSharedSentryEvent());
	});

	Describe("Event params", [this]()
	{
		It("should retain their values", [this]()
		{
			const FString TestMessage = FString(TEXT("Test event message"));

			SentryEvent->SetLevel(ESentryLevel::Fatal);
			SentryEvent->SetMessage(TestMessage);

			TestEqual("Event level", SentryEvent->GetLevel(), ESentryLevel::Fatal);
			TestEqual("Event message", SentryEvent->GetMessage(), TestMessage);
			TestFalse("Event ID is non-empty", SentryEvent->GetId().IsEmpty());
		});
	});

	Describe("Event fingerprint", [this]()
	{
		It("should retain its value", [this]()
		{
			TArray<FString> InFingerprint = { TEXT("F1"), TEXT("F2"), TEXT("F3") };

			SentryEvent->SetFingerprint(InFingerprint);
			TArray<FString> OutFingerprint = SentryEvent->GetFingerprint();

			TestEqual("Fingerprint elements count", OutFingerprint.Num(), InFingerprint.Num());
			TestEqual("Fingerprint first element", OutFingerprint[0], InFingerprint[0]);
			TestEqual("Fingerprint second element", OutFingerprint[1], InFingerprint[1]);
			TestEqual("Fingerprint third element", OutFingerprint[2], InFingerprint[2]);
		});

		It("can be emptry", [this]()
		{
			TArray<FString> InFingerprint = {};

			SentryEvent->SetFingerprint(InFingerprint);
			TArray<FString> OutFingerprint = SentryEvent->GetFingerprint();

			TestEqual("Fingerprint elements count", OutFingerprint.Num(), InFingerprint.Num());
			TestEqual("Fingerprint is empty", OutFingerprint.Num(), 0);
		});
	});

	Describe("Event tags", [this]()
	{
		It("can be added and removed", [this]()
		{
			TestEqual("Tags are empty by default", SentryEvent->GetTags().Num(), 0);

			FString NonExistingTag;
			TestFalse("Can't get any tag while these are empty (Try)", SentryEvent->TryGetTag(TEXT("TagsKey0"), NonExistingTag));

			SentryEvent->SetTag(TEXT("TagsKey1"), TEXT("TagsVal1"));
			SentryEvent->SetTag(TEXT("TagsKey2"), TEXT("TagsVal2"));

			TestEqual("There are two tags adding them as individual items", SentryEvent->GetTags().Num(), 2);

			TestEqual("First tag retains its original value", SentryEvent->GetTag(TEXT("TagsKey1")), TEXT("TagsVal1"));
			TestEqual("Second tag retains its original value", SentryEvent->GetTag(TEXT("TagsKey2")), TEXT("TagsVal2"));

			TestEqual("Can't get non-existent tag", SentryEvent->GetTag(TEXT("TagsKey3")), TEXT(""));

			SentryEvent->RemoveTag(TEXT("TagsKey1"));
			TestFalse("Can't get first tag after it was removed (Try)", SentryEvent->TryGetTag(TEXT("TagsKey1"), NonExistingTag));
			TestEqual("Can't get first tag after it was removed", SentryEvent->GetTag(TEXT("TagsKey1")), TEXT(""));
			TestEqual("One tag left", SentryEvent->GetTags().Num(), 1);

			SentryEvent->RemoveTag(TEXT("TagsKey2"));
			TestFalse("Can't get second tag after it was removed (Try)", SentryEvent->TryGetTag(TEXT("TagsKey2"), NonExistingTag));
			TestEqual("Can't get second tag after it was removed", SentryEvent->GetTag(TEXT("TagsKey2")), TEXT(""));
			TestEqual("No tags left", SentryEvent->GetTags().Num(), 0);

			TMap<FString, FString> TestTags;
			TestTags.Add(TEXT("TagsKey3"), TEXT("TagsVal3"));
			TestTags.Add(TEXT("TagsKey4"), TEXT("TagsVal4"));

			SentryEvent->SetTags(TestTags);
			TestEqual("There are two tags after adding them as map", SentryEvent->GetTags().Num(), 2);

			TestEqual("Third tag retain its original value", SentryEvent->GetTag(TEXT("TagsKey3")), TEXT("TagsVal3"));
			TestEqual("Fourth tag retain its original value", SentryEvent->GetTag(TEXT("TagsKey4")), TEXT("TagsVal4"));

			SentryEvent->SetTags(TMap<FString, FString>());
			TestEqual("There are no tags after setting an empty map", SentryEvent->GetTags().Num(), 0);
		});
	});

	Describe("Event contexts", [this]()
	{
		It("can be added and removed", [this]()
		{
			const TArray<FSentryVariant>& VariantArray = {
				TEXT("Hello nested array")
			};

			const TMap<FString, FSentryVariant>& VariantMap = {
				{ TEXT("Key"), TEXT("Hello nested map") }
			};

			TMap<FString, FSentryVariant> InTestContext;
			InTestContext.Add(TEXT("ContextKey1"), TEXT("ContextVal1"));
			InTestContext.Add(TEXT("ContextKey2"), 1234);
			InTestContext.Add(TEXT("ContextKey3"), 222.333f);
			InTestContext.Add(TEXT("ContextKey4"), true);
			InTestContext.Add(TEXT("ContextKey5"), VariantArray);
			InTestContext.Add(TEXT("ContextKey6"), VariantMap);

			SentryEvent->SetContext(TEXT("TestContext1"), InTestContext);

			TMap<FString, FSentryVariant> OutTestContext = SentryEvent->GetContext(TEXT("TestContext1"));

			TestEqual("Context exist after it was added", OutTestContext.Num(), 6);

			TestEqual("Context retains its string value", OutTestContext[TEXT("ContextKey1")], InTestContext[TEXT("ContextKey1")]);
			TestEqual("Context retains its integer value", OutTestContext[TEXT("ContextKey2")], InTestContext[TEXT("ContextKey2")]);
			TestEqual("Context retains its float value", OutTestContext[TEXT("ContextKey3")], InTestContext[TEXT("ContextKey3")]);
			TestEqual("Context retains its bool value", OutTestContext[TEXT("ContextKey4")], InTestContext[TEXT("ContextKey4")]);
			TestEqual("Context retains its array value", OutTestContext[TEXT("ContextKey5")], InTestContext[TEXT("ContextKey5")]);
			TestEqual("Context retains its map value", OutTestContext[TEXT("ContextKey6")], InTestContext[TEXT("ContextKey6")]);

			SentryEvent->RemoveContext(TEXT("TestContext1"));

			TMap<FString, FSentryVariant> NonExistingContext;
			TestFalse("No context with given key available after it was removed (Try)", SentryEvent->TryGetContext(TEXT("TestContext1"), NonExistingContext));
			TestEqual("No context with given key available after it was removed", SentryEvent->GetContext(TEXT("TestContext1")).Num(), 0);
		});
	});

	Describe("Event extras", [this]()
	{
		It("can be added and removed", [this]()
		{
			TestEqual("Extras are empty by default", SentryEvent->GetExtras().Num(), 0);

			FSentryVariant NonExistingExtra;
			TestFalse("Can't get any extra while these are empty (Try)", SentryEvent->TryGetExtra(TEXT("ExtraKey0"), NonExistingExtra));
			TestEqual("Can't get any extra while these are empty", SentryEvent->GetExtra(TEXT("ExtraKey0")).GetType(), ESentryVariantType::Empty);

			SentryEvent->SetExtra(TEXT("ExtraKey1"), TEXT("ExtraVal1"));
			SentryEvent->SetExtra(TEXT("ExtraKey2"), TEXT("ExtraVal2"));

			TestEqual("There are two extras after adding them as individual items", SentryEvent->GetExtras().Num(), 2);

			TestEqual("First extra retains its original value", SentryEvent->GetExtra(TEXT("ExtraKey1")).GetValue<FString>(), TEXT("ExtraVal1"));
			TestEqual("Second extra retains its original value", SentryEvent->GetExtra(TEXT("ExtraKey2")).GetValue<FString>(), TEXT("ExtraVal2"));

			TestEqual("Can't get non-existent extra", SentryEvent->GetExtra(TEXT("ExtraKey3")).GetType(), ESentryVariantType::Empty);

			SentryEvent->RemoveExtra(TEXT("ExtraKey1"));
			TestFalse("Can't get first extra after it was removed (Try)", SentryEvent->TryGetExtra(TEXT("ExtraKey1"), NonExistingExtra));
			TestEqual("Can't get first extra after it was removed", SentryEvent->GetExtra(TEXT("ExtraKey1")).GetType(), ESentryVariantType::Empty);
			TestEqual("One extra left", SentryEvent->GetExtras().Num(), 1);

			SentryEvent->RemoveExtra(TEXT("ExtraKey2"));
			TestFalse("Can't get second extra after it was removed (Try)", SentryEvent->TryGetExtra(TEXT("TExtraKey2"), NonExistingExtra));
			TestEqual("Can't get second extra after it was removed", SentryEvent->GetExtra(TEXT("TExtraKey2")).GetType(), ESentryVariantType::Empty);
			TestEqual("No extras left", SentryEvent->GetExtras().Num(), 0);

			TMap<FString, FSentryVariant> TestExtra;
			TestExtra.Add(TEXT("ExtraKey3"), TEXT("ExtraVal3"));
			TestExtra.Add(TEXT("ExtraKey4"), TEXT("ExtraVal4"));

			SentryEvent->SetExtras(TestExtra);
			TestEqual("There are two extras after adding them as map", SentryEvent->GetExtras().Num(), 2);

			TestEqual("Third extra retains its original value", SentryEvent->GetExtra(TEXT("ExtraKey3")).GetValue<FString>(), TEXT("ExtraVal3"));
			TestEqual("Fourth extra retains its original value", SentryEvent->GetExtra(TEXT("ExtraKey4")).GetValue<FString>(), TEXT("ExtraVal4"));

			SentryEvent->SetExtras(TMap<FString, FSentryVariant>());
			TestEqual("There are no extras after setting an empty map", SentryEvent->GetExtras().Num(), 0);
		});
	});
}

#endif