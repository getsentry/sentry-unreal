// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/Variant.h"

#include "SentryVariant.generated.h"

struct FSentryVariant;

UENUM(BlueprintType)
enum class ESentryVariantType : uint8
{
	Empty,
	Integer,
	Float,
	Bool,
	String,
	Array,
	Map
};

template<>
struct TVariantTraits<TArray<FSentryVariant>>
{
	static constexpr EVariantTypes GetType() { return EVariantTypes::Custom; }
};

template<>
struct TVariantTraits<TMap<FString, FSentryVariant>>
{
	static constexpr EVariantTypes GetType() { return EVariantTypes::Custom; }
};

/**
 * Blueprint-exposed struct that can represent multiple types.
 */
USTRUCT(BlueprintType)
struct SENTRY_API FSentryVariant
{
	GENERATED_BODY()

	FSentryVariant();
	FSentryVariant(int32 InValue);
	FSentryVariant(float InValue);
	FSentryVariant(bool InValue);
	FSentryVariant(const FString& InValue);
	FSentryVariant(const TCHAR* InValue);
	FSentryVariant(const TArray<FSentryVariant>& InValue);
	FSentryVariant(const TMap<FString, FSentryVariant>& InValue);

	template<typename T>
	T GetValue() const
	{
		return Value.GetValue<T>();
	}

	operator int32() const
	{
		return GetValue<int32>();
	}

	operator float() const
	{
		return GetValue<float>();
	}

	operator bool() const
	{
		return GetValue<bool>();
	}

	operator FString() const
	{
		return GetValue<FString>();
	}

	operator TArray<FSentryVariant>() const
	{
		return GetValue<TArray<FSentryVariant>>();
	}

	operator TMap<FString, FSentryVariant>() const
	{
		return GetValue<TMap<FString, FSentryVariant>>();
	}

	ESentryVariantType GetType() const
	{
		return Type;
	}

	bool operator==(const FSentryVariant& Other) const
	{
		return ((Type == Other.Type) && (Value == Other.Value));
	}

	bool operator!=(const FSentryVariant& Other) const
	{
		return ((Type != Other.Type) || (Value != Other.Value));
	}

	friend FArchive& operator<<(FArchive& Ar, FSentryVariant& Variant)
	{
		return Ar << Variant.Type << Variant.Value;
	}

private:
	ESentryVariantType Type;
	FVariant Value;
};

UCLASS()
class SENTRY_API USentryVariantHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Makes a new variant from the specified integer. */
	UFUNCTION(BlueprintPure, Category = "Sentry|Variant", meta = (NativeMakeFunc))
	static FSentryVariant MakeSentryVariantFromInteger(int32 Value);

	/** Makes a new variant from the specified float. */
	UFUNCTION(BlueprintPure, Category = "Sentry|Variant", meta = (NativeMakeFunc))
	static FSentryVariant MakeSentryVariantFromFloat(float Value);

	/** Makes a new variant from the specified bool. */
	UFUNCTION(BlueprintPure, Category = "Sentry|Variant", meta = (NativeMakeFunc))
	static FSentryVariant MakeSentryVariantFromBool(bool Value);

	/** Makes a new variant from the specified string. */
	UFUNCTION(BlueprintPure, Category = "Sentry|Variant", meta = (NativeMakeFunc))
	static FSentryVariant MakeSentryVariantFromString(const FString& Value);

	/** Makes a new variant from the specified array. */
	UFUNCTION(BlueprintPure, Category = "Sentry|Variant", meta = (NativeMakeFunc))
	static FSentryVariant MakeSentryVariantFromArray(const TArray<FSentryVariant>& Value);

	/** Makes a new variant from the specified map. */
	UFUNCTION(BlueprintPure, Category = "Sentry|Variant", meta = (NativeMakeFunc))
	static FSentryVariant MakeSentryVariantFromMap(const TMap<FString, FSentryVariant>& Value);

	/**
	 * Converts integer to variant.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Integer To Sentry Variant", CompactNodeTitle = "->", BlueprintAutocast), Category = "Sentry|Variant")
	static FSentryVariant Conv_IntToFSentryVariant(int32 Value);

	/**
	 * Converts float to variant.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Float To Sentry Variant", CompactNodeTitle = "->", BlueprintAutocast), Category = "Sentry|Variant")
	static FSentryVariant Conv_FloatToFSentryVariant(float Value);

	/**
	 * Converts bool to variant.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Bool To Sentry Variant", CompactNodeTitle = "->", BlueprintAutocast), Category = "Sentry|Variant")
	static FSentryVariant Conv_BoolToFSentryVariant(bool Value);

	/**
	 * Converts string to variant.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "String To Sentry Variant", CompactNodeTitle = "->", BlueprintAutocast), Category = "Sentry|Variant")
	static FSentryVariant Conv_StringToFSentryVariant(const FString& Value);

	/**
	 * Converts array to variant.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Array To Sentry Variant", CompactNodeTitle = "->", BlueprintAutocast), Category = "Sentry|Variant")
	static FSentryVariant Conv_ArrayToFSentryVariant(const TArray<FSentryVariant>& Value);

	/**
	 * Converts map to variant.
	 */
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Map To Sentry Variant", CompactNodeTitle = "->", BlueprintAutocast), Category = "Sentry|Variant")
	static FSentryVariant Conv_MapToFSentryVariant(const TMap<FString, FSentryVariant>& Value);

	/**
	 * Get variant value type
	 *
	 * @return - Value type.
	 */
	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "Type"), Category = "Sentry|Variant")
	static ESentryVariantType GetType(const FSentryVariant& Variant);

	/**
	 * Get variant value
	 *
	 * @return - Integer value.
	 */
	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "Integer"), Category = "Sentry|Variant")
	static int32 GetInteger(const FSentryVariant& Variant);

	/**
	 * Try to get variant value
	 *
	 * @return - True if value retrieval was successful.
	 */
	UFUNCTION(BlueprintPure, Category = "Sentry|Variant")
	static bool TryGetInteger(const FSentryVariant& Variant, int32& Value);

	/**
	 * Get variant value
	 *
	 * @return - Float value.
	 */
	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "Float"), Category = "Sentry|Variant")
	static float GetFloat(const FSentryVariant& Variant);

	/**
	 * Try to get variant value
	 *
	 * @return - True if value retrieval was successful.
	 */
	UFUNCTION(BlueprintPure, Category = "Sentry|Variant")
	static bool TryGetFloat(const FSentryVariant& Variant, float& Value);

	/**
	 * Get variant value
	 *
	 * @return - Boolean value.
	 */
	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "Bool"), Category = "Sentry|Variant")
	static bool GetBool(const FSentryVariant& Variant);

	/**
	 * Try to get variant value
	 *
	 * @return - True if value retrieval was successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry|Variant")
	static bool TryGetBool(const FSentryVariant& Variant, bool& Value);

	/**
	 * Get variant value
	 *
	 * @return - String value.
	 */
	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "String"), Category = "Sentry|Variant")
	static FString GetString(const FSentryVariant& Variant);

	/**
	 * Try to get variant value
	 *
	 * @return - True if value retrieval was successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry|Variant")
	static bool TryGetString(const FSentryVariant& Variant, FString& Value);

	/**
	 * Get variant value
	 *
	 * @return - Array value.
	 */
	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "Array"), Category = "Sentry|Variant")
	static TArray<FSentryVariant> GetArray(const FSentryVariant& Variant);

	/**
	 * Try to get variant value
	 *
	 * @return - True if value retrieval was successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry|Variant")
	static bool TryGetArray(const FSentryVariant& Variant, TArray<FSentryVariant>& Value);

	/**
	 * Get variant value
	 *
	 * @return - Map value.
	 */
	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "Map"), Category = "Sentry|Variant")
	static TMap<FString, FSentryVariant> GetMap(const FSentryVariant& Variant);

	/**
	 * Try to get variant value
	 *
	 * @return - True if value retrieval was successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry|Variant")
	static bool TryGetMap(const FSentryVariant& Variant, TMap<FString, FSentryVariant>& Value);
};
