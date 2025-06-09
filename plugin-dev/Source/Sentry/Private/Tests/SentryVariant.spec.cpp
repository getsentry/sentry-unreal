// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "SentryVariant.h"
#include "SentryTests.h"

#if WITH_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(SentryVariantSpec, "Sentry.SentryVariant", EAutomationTestFlags::ProductFilter | SentryApplicationContextMask)
END_DEFINE_SPEC(SentryVariantSpec)

void SentryVariantSpec::Define()
{
	Describe("Variant", [this]()
	{
		It("should support different types", [this]()
		{
			FSentryVariant EmptyVariant;
			TestTrue("Is empty", EmptyVariant.Type == ESentryVariantType::Empty);

			FSentryVariant IntegerVariant(1234);
			TestTrue("Is integer", IntegerVariant.Type == ESentryVariantType::Integer);
			TestEqual("Integer value", IntegerVariant.GetValue<int32>(), 1234);

			FSentryVariant FloatVariant(222.333f);
			TestTrue("Is float", FloatVariant.Type == ESentryVariantType::Float);
			TestEqual("Float value", FloatVariant.GetValue<float>(), 222.333f);

			FSentryVariant BoolVariant(true);
			TestTrue("Is bool", BoolVariant.Type == ESentryVariantType::Bool);
			TestEqual("Bool value", BoolVariant.GetValue<bool>(), true);

			FSentryVariant StringVariant(TEXT("Hello variant"));
			TestTrue("Is string", StringVariant.Type == ESentryVariantType::String);
			TestEqual("String value", StringVariant.GetValue<FString>(), TEXT("Hello variant"));

			const TArray<FSentryVariant>& TestArray = {
				1234,
				222.333f,
				true,
				TEXT("Hello array")
			};

			FSentryVariant ArrayVariant(TestArray);
			TestTrue("Is array", ArrayVariant.Type == ESentryVariantType::Array);
			TestEqual("Array values", ArrayVariant.GetValue<TArray<FSentryVariant>>(), TestArray);

			const TMap<FString, FSentryVariant>& TestMap = {
				{ TEXT("Key1"), 1234 },
				{ TEXT("Key2"), 222.333f },
				{ TEXT("Key3"), true },
				{ TEXT("Key4"), TEXT("Hello map") }
			};

			FSentryVariant MapVariant(TestMap);
			TestTrue("Is map", MapVariant.Type == ESentryVariantType::Map);
			TestEqual("Map value 1", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("Key1")], TestMap[TEXT("Key1")]);
			TestEqual("Map value 2", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("Key2")], TestMap[TEXT("Key2")]);
			TestEqual("Map value 3", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("Key3")], TestMap[TEXT("Key3")]);
			TestEqual("Map value 4", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("Key4")], TestMap[TEXT("Key4")]);
		});

		It("should support nested arrays", [this]()
		{
			const TArray<FSentryVariant>& Array1 = {
				TEXT("Hello nested array")
			};

			const TArray<FSentryVariant>& Array2 = {
				TEXT("Hello parent array"),
				Array1
			};

			FSentryVariant ArrayVariant(Array2);

			TestTrue("Is array", ArrayVariant.Type == ESentryVariantType::Array);

			const TArray<FSentryVariant>& ParentArray = ArrayVariant.GetValue<TArray<FSentryVariant>>();

			TestTrue("Has string value", ParentArray[0].Type == ESentryVariantType::String);
			TestTrue("Has nested array", ParentArray[1].Type == ESentryVariantType::Array);

			const TArray<FSentryVariant>& NestedArray = ParentArray[1].GetValue<TArray<FSentryVariant>>();

			TestTrue("Nested array has string", NestedArray[0].Type == ESentryVariantType::String);
			TestEqual("Nested array string value", NestedArray[0].GetValue<FString>(), TEXT("Hello nested array"));
		});

		It("should support nested maps", [this]()
		{
			const TMap<FString, FSentryVariant>& Map1 = {
				{ TEXT("Key1"), TEXT("Hello nested map") }
			};

			const TMap<FString, FSentryVariant>& Map2 = {
				{ TEXT("StringKey"), TEXT("Hello parent map") },
				{ TEXT("MapKey"), Map1 },
			};

			FSentryVariant MapVariant(Map2);

			TestTrue("Is array", MapVariant.Type == ESentryVariantType::Map);

			const TMap<FString, FSentryVariant>& ParentMap = MapVariant.GetValue<TMap<FString, FSentryVariant>>();

			TestTrue("Has string value", ParentMap[TEXT("StringKey")].Type == ESentryVariantType::String);
			TestTrue("Has nested map", ParentMap[TEXT("MapKey")].Type == ESentryVariantType::Map);

			const TMap<FString, FSentryVariant>& NestedMap = ParentMap[TEXT("MapKey")].GetValue<TMap<FString, FSentryVariant>>();

			TestTrue("Nested map has string", NestedMap[TEXT("Key1")].Type == ESentryVariantType::String);
			TestEqual("Nested map string value", NestedMap[TEXT("Key1")].GetValue<FString>(), TEXT("Hello nested map"));
		});
	});
}

#endif
