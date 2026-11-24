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
			TestTrue("Is empty", EmptyVariant.GetType() == ESentryVariantType::Empty);

			FSentryVariant IntegerVariant(1234);
			TestTrue("Is integer", IntegerVariant.GetType() == ESentryVariantType::Integer);
			TestEqual("Integer value", IntegerVariant.GetValue<int32>(), 1234);
			int32 IntegerValue = IntegerVariant;
			TestEqual("Integer value", IntegerValue, 1234);

			FSentryVariant FloatVariant(222.333f);
			TestTrue("Is float", FloatVariant.GetType() == ESentryVariantType::Float);
			TestEqual("Float value", FloatVariant.GetValue<float>(), 222.333f);
			float FloatValue = FloatVariant;
			TestEqual("Float value", FloatValue, 222.333f);

			FSentryVariant BoolVariant(true);
			TestTrue("Is bool", BoolVariant.GetType() == ESentryVariantType::Bool);
			TestEqual("Bool value", BoolVariant.GetValue<bool>(), true);
			bool BoolValue = BoolVariant;
			TestEqual("Bool value", BoolValue, true);

			FSentryVariant StringVariant(TEXT("Hello variant"));
			TestTrue("Is string", StringVariant.GetType() == ESentryVariantType::String);
			TestEqual("String value", StringVariant.GetValue<FString>(), TEXT("Hello variant"));
			FString StringValue = StringVariant;
			TestEqual("String value", StringValue, TEXT("Hello variant"));

			const TArray<FSentryVariant>& TestArray = {
				1234,
				222.333f,
				true,
				TEXT("Hello array")
			};

			FSentryVariant ArrayVariant(TestArray);
			TestTrue("Is array", ArrayVariant.GetType() == ESentryVariantType::Array);
			TestEqual("Array values", ArrayVariant.GetValue<TArray<FSentryVariant>>(), TestArray);
			TArray<FSentryVariant> ArrayValue = ArrayVariant;
			TestEqual("Array values", ArrayValue, TestArray);

			const TMap<FString, FSentryVariant>& TestMap = {
				{ TEXT("Key1"), 1234 },
				{ TEXT("Key2"), 222.333f },
				{ TEXT("Key3"), true },
				{ TEXT("Key4"), TEXT("Hello map") }
			};

			FSentryVariant MapVariant(TestMap);
			TestTrue("Is map", MapVariant.GetType() == ESentryVariantType::Map);
			TestEqual("Map value 1", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("Key1")], TestMap[TEXT("Key1")]);
			TestEqual("Map value 2", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("Key2")], TestMap[TEXT("Key2")]);
			TestEqual("Map value 3", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("Key3")], TestMap[TEXT("Key3")]);
			TestEqual("Map value 4", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("Key4")], TestMap[TEXT("Key4")]);
			TMap<FString, FSentryVariant> MapValue = MapVariant;
			TestEqual("Map value 1", MapValue[TEXT("Key1")], TestMap[TEXT("Key1")]);
			TestEqual("Map value 2", MapValue[TEXT("Key2")], TestMap[TEXT("Key2")]);
			TestEqual("Map value 3", MapValue[TEXT("Key3")], TestMap[TEXT("Key3")]);
			TestEqual("Map value 4", MapValue[TEXT("Key4")], TestMap[TEXT("Key4")]);
		});

		It("should support copy operations", [this]()
		{
			FSentryVariant Original(42);
			FSentryVariant Copy(Original);

			TestTrue("Copy has same type", Copy.GetType() == Original.GetType());
			TestEqual("Copy has same value", Copy.GetValue<int32>(), Original.GetValue<int32>());

			FSentryVariant Assigned;
			Assigned = Original;

			TestTrue("Assignment has same type", Assigned.GetType() == Original.GetType());
			TestEqual("Assignment has same value", Assigned.GetValue<int32>(), Original.GetValue<int32>());
		});

		It("should handle equality comparisons", [this]()
		{
			FSentryVariant Var1(123);
			FSentryVariant Var2(123);
			FSentryVariant Var3(456);
			FSentryVariant Var4(123.0f);

			TestTrue("Same values are equal", Var1 == Var2);
			TestFalse("Different values are not equal", Var1 == Var3);
			TestFalse("Different types are not equal", Var1 == Var4);

			TestFalse("Same values are not unequal", Var1 != Var2);
			TestTrue("Different values are unequal", Var1 != Var3);
			TestTrue("Different types are unequal", Var1 != Var4);
		});

		It("should handle empty containers", [this]()
		{
			TArray<FSentryVariant> EmptyArray;
			FSentryVariant ArrayVariant(EmptyArray);

			TestTrue("Is array", ArrayVariant.GetType() == ESentryVariantType::Array);
			TestEqual("Array is empty", ArrayVariant.GetValue<TArray<FSentryVariant>>().Num(), 0);

			TMap<FString, FSentryVariant> EmptyMap;
			FSentryVariant MapVariant(EmptyMap);

			TestTrue("Is map", MapVariant.GetType() == ESentryVariantType::Map);
			TestEqual("Map is empty", MapVariant.GetValue<TMap<FString, FSentryVariant>>().Num(), 0);
		});

		It("should support nested containers", [this]()
		{
			const TArray<FSentryVariant>& NestedArray = {
				TEXT("Hello nested array")
			};

			const TMap<FString, FSentryVariant>& NestedMap = {
				{ TEXT("Key"), TEXT("Hello nested map") }
			};

			const TArray<FSentryVariant>& ParentArray = {
				NestedArray,
				NestedMap
			};

			const TMap<FString, FSentryVariant>& ParentMap = {
				{ TEXT("ParentKey1"), NestedArray },
				{ TEXT("ParentKey2"), NestedMap },
			};

			FSentryVariant ArrayVariant(ParentArray);

			TestTrue("Is array", ArrayVariant.GetType() == ESentryVariantType::Array);
			TestTrue("Array has nested array", ArrayVariant.GetValue<TArray<FSentryVariant>>()[0].GetType() == ESentryVariantType::Array);
			TestTrue("Array has nested map", ArrayVariant.GetValue<TArray<FSentryVariant>>()[1].GetType() == ESentryVariantType::Map);

			FSentryVariant MapVariant(ParentMap);

			TestTrue("Is array", MapVariant.GetType() == ESentryVariantType::Map);
			TestTrue("Map has nested array", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("ParentKey1")].GetType() == ESentryVariantType::Array);
			TestTrue("Map has nested map", MapVariant.GetValue<TMap<FString, FSentryVariant>>()[TEXT("ParentKey2")].GetType() == ESentryVariantType::Map);
		});

		It("should retain nested array values", [this]()
		{
			const TArray<FSentryVariant>& Array1 = {
				TEXT("Hello nested array 1"),
				TEXT("Hello nested array 2")
			};

			const TArray<FSentryVariant>& Array2 = {
				FSentryVariant(Array1)
			};

			FSentryVariant ArrayVariant(Array2);

			TestTrue("Is array", ArrayVariant.GetType() == ESentryVariantType::Array);

			const TArray<FSentryVariant>& ParentArray = ArrayVariant.GetValue<TArray<FSentryVariant>>();

			TestTrue("Has nested array", ParentArray[0].GetType() == ESentryVariantType::Array);

			const TArray<FSentryVariant>& NestedArray = ParentArray[0].GetValue<TArray<FSentryVariant>>();

			TestEqual("Nested array string value 1", NestedArray[0].GetValue<FString>(), TEXT("Hello nested array 1"));
			TestEqual("Nested array string value 2", NestedArray[1].GetValue<FString>(), TEXT("Hello nested array 2"));
		});

		It("should retain nested map values", [this]()
		{
			const TMap<FString, FSentryVariant>& Map1 = {
				{ TEXT("Key1"), TEXT("Hello nested map 1") },
				{ TEXT("Key2"), TEXT("Hello nested map 2") }
			};

			const TMap<FString, FSentryVariant>& Map2 = {
				{ TEXT("MapKey"), Map1 }
			};

			FSentryVariant MapVariant(Map2);

			TestTrue("Is array", MapVariant.GetType() == ESentryVariantType::Map);

			const TMap<FString, FSentryVariant>& ParentMap = MapVariant.GetValue<TMap<FString, FSentryVariant>>();

			TestTrue("Has nested map", ParentMap[TEXT("MapKey")].GetType() == ESentryVariantType::Map);

			const TMap<FString, FSentryVariant>& NestedMap = ParentMap[TEXT("MapKey")].GetValue<TMap<FString, FSentryVariant>>();

			TestEqual("Nested map string value 1", NestedMap[TEXT("Key1")].GetValue<FString>(), TEXT("Hello nested map 1"));
			TestEqual("Nested map string value 2", NestedMap[TEXT("Key2")].GetValue<FString>(), TEXT("Hello nested map 2"));
		});
	});

	Describe("Variant blueprint helper", [this]()
	{
		It("should create variants from make functions", [this]()
		{
			FSentryVariant IntVariant = USentryVariantHelper::MakeSentryVariantFromInteger(42);
			TestTrue("Make integer variant", IntVariant.GetType() == ESentryVariantType::Integer);
			TestEqual("Make integer value", IntVariant.GetValue<int32>(), 42);

			FSentryVariant FloatVariant = USentryVariantHelper::MakeSentryVariantFromFloat(3.14f);
			TestTrue("Make float variant", FloatVariant.GetType() == ESentryVariantType::Float);
			TestEqual("Make float value", FloatVariant.GetValue<float>(), 3.14f);

			FSentryVariant BoolVariant = USentryVariantHelper::MakeSentryVariantFromBool(true);
			TestTrue("Make bool variant", BoolVariant.GetType() == ESentryVariantType::Bool);
			TestEqual("Make bool value", BoolVariant.GetValue<bool>(), true);

			FSentryVariant StringVariant = USentryVariantHelper::MakeSentryVariantFromString(TEXT("Hello"));
			TestTrue("Make string variant", StringVariant.GetType() == ESentryVariantType::String);
			TestEqual("Make string value", StringVariant.GetValue<FString>(), TEXT("Hello"));

			TArray<FSentryVariant> TestArray = { 1, 2, 3 };
			FSentryVariant ArrayVariant = USentryVariantHelper::MakeSentryVariantFromArray(TestArray);
			TestTrue("Make array variant", ArrayVariant.GetType() == ESentryVariantType::Array);
			TestEqual("Make array size", ArrayVariant.GetValue<TArray<FSentryVariant>>().Num(), 3);

			TMap<FString, FSentryVariant> TestMap = { { TEXT("key"), TEXT("value") } };
			FSentryVariant MapVariant = USentryVariantHelper::MakeSentryVariantFromMap(TestMap);
			TestTrue("Make map variant", MapVariant.GetType() == ESentryVariantType::Map);
			TestEqual("Make map size", MapVariant.GetValue<TMap<FString, FSentryVariant>>().Num(), 1);
		});

		It("should get variant types correctly", [this]()
		{
			FSentryVariant IntVariant(123);
			FSentryVariant FloatVariant(1.23f);
			FSentryVariant BoolVariant(false);
			FSentryVariant StringVariant(TEXT("test"));
			FSentryVariant ArrayVariant(TArray<FSentryVariant>{ 1, 2 });
			FSentryVariant MapVariant(TMap<FString, FSentryVariant>{ { TEXT("k"), TEXT("v") } });

			TestTrue("Get integer type", USentryVariantHelper::GetType(IntVariant) == ESentryVariantType::Integer);
			TestTrue("Get float type", USentryVariantHelper::GetType(FloatVariant) == ESentryVariantType::Float);
			TestTrue("Get bool type", USentryVariantHelper::GetType(BoolVariant) == ESentryVariantType::Bool);
			TestTrue("Get string type", USentryVariantHelper::GetType(StringVariant) == ESentryVariantType::String);
			TestTrue("Get array type", USentryVariantHelper::GetType(ArrayVariant) == ESentryVariantType::Array);
			TestTrue("Get map type", USentryVariantHelper::GetType(MapVariant) == ESentryVariantType::Map);
		});

		It("should get variant values correctly", [this]()
		{
			FSentryVariant IntVariant(456);
			FSentryVariant FloatVariant(4.56f);
			FSentryVariant BoolVariant(true);
			FSentryVariant StringVariant(TEXT("get test"));

			TestEqual("Get integer value", USentryVariantHelper::GetInteger(IntVariant), 456);
			TestEqual("Get float value", USentryVariantHelper::GetFloat(FloatVariant), 4.56f);
			TestEqual("Get bool value", USentryVariantHelper::GetBool(BoolVariant), true);
			TestEqual("Get string value", USentryVariantHelper::GetString(StringVariant), TEXT("get test"));

			TArray<FSentryVariant> TestArray = { 10, 20, 30 };
			FSentryVariant ArrayVariant(TestArray);
			TArray<FSentryVariant> RetrievedArray = USentryVariantHelper::GetArray(ArrayVariant);
			TestEqual("Get array size", RetrievedArray.Num(), 3);
			TestEqual("Get array element", RetrievedArray[0].GetValue<int32>(), 10);

			TMap<FString, FSentryVariant> TestMap = { { TEXT("key1"), 100 }, { TEXT("key2"), TEXT("value2") } };
			FSentryVariant MapVariant(TestMap);
			TMap<FString, FSentryVariant> RetrievedMap = USentryVariantHelper::GetMap(MapVariant);
			TestEqual("Get map size", RetrievedMap.Num(), 2);
			TestEqual("Get map value", RetrievedMap[TEXT("key1")].GetValue<int32>(), 100);
		});

		It("should handle try get functions correctly", [this]()
		{
			FSentryVariant IntVariant(789);
			FSentryVariant StringVariant(TEXT("try test"));

			int32 IntValue;
			bool bGotInt = USentryVariantHelper::TryGetInteger(IntVariant, IntValue);
			TestTrue("Try get integer success", bGotInt);
			TestEqual("Try get integer value", IntValue, 789);

			float FloatValue;
			bool bGotFloat = USentryVariantHelper::TryGetFloat(IntVariant, FloatValue);
			TestFalse("Try get float from integer fails", bGotFloat);

			FString StringValue;
			bool bGotString = USentryVariantHelper::TryGetString(StringVariant, StringValue);
			TestTrue("Try get string success", bGotString);
			TestEqual("Try get string value", StringValue, TEXT("try test"));

			bool BoolValue;
			bool bGotBool = USentryVariantHelper::TryGetBool(StringVariant, BoolValue);
			TestFalse("Try get bool from string fails", bGotBool);

			TArray<FSentryVariant> TestArray = { 1, 2 };
			FSentryVariant ArrayVariant(TestArray);
			TArray<FSentryVariant> ArrayValue;
			bool bGotArray = USentryVariantHelper::TryGetArray(ArrayVariant, ArrayValue);
			TestTrue("Try get array success", bGotArray);
			TestEqual("Try get array size", ArrayValue.Num(), 2);

			TMap<FString, FSentryVariant> TestMap = { { TEXT("test"), 42 } };
			FSentryVariant MapVariant(TestMap);
			TMap<FString, FSentryVariant> MapValue;
			bool bGotMap = USentryVariantHelper::TryGetMap(MapVariant, MapValue);
			TestTrue("Try get map success", bGotMap);
			TestEqual("Try get map size", MapValue.Num(), 1);

			TMap<FString, FSentryVariant> WrongMapValue;
			bool bGotWrongMap = USentryVariantHelper::TryGetMap(IntVariant, WrongMapValue);
			TestFalse("Try get map from integer fails", bGotWrongMap);
		});

		It("should handle empty variant", [this]()
		{
			FSentryVariant EmptyVariant;
			TestTrue("Empty variant type", USentryVariantHelper::GetType(EmptyVariant) == ESentryVariantType::Empty);

			int32 IntValue;
			bool bGotInt = USentryVariantHelper::TryGetInteger(EmptyVariant, IntValue);
			TestFalse("Try get integer from empty fails", bGotInt);

			FString StringValue;
			bool bGotString = USentryVariantHelper::TryGetString(EmptyVariant, StringValue);
			TestFalse("Try get string from empty fails", bGotString);

			TArray<FSentryVariant> ArrayValue;
			bool bGotArray = USentryVariantHelper::TryGetArray(EmptyVariant, ArrayValue);
			TestFalse("Try get array from empty fails", bGotArray);
		});
	});
}

#endif
