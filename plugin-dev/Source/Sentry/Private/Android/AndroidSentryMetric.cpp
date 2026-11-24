// Copyright (c) 2025 Sentry. All Rights Reserved.

#include "AndroidSentryMetric.h"

#include "Infrastructure/AndroidSentryConverters.h"
#include "Infrastructure/AndroidSentryJavaClasses.h"

FAndroidSentryMetric::FAndroidSentryMetric()
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryMetricsEvent, "()V")
{
	SetupClassMethods();
}

FAndroidSentryMetric::FAndroidSentryMetric(jobject metricEvent)
	: FSentryJavaObjectWrapper(SentryJavaClasses::SentryMetricsEvent, metricEvent)
{
	SetupClassMethods();
}

void FAndroidSentryMetric::SetupClassMethods()
{
	SetNameMethod = GetMethod("setName", "(Ljava/lang/String;)V");
	GetNameMethod = GetMethod("getName", "()Ljava/lang/String;");
	SetTypeMethod = GetMethod("setType", "(Ljava/lang/String;)V");
	GetTypeMethod = GetMethod("getType", "()Ljava/lang/String;");
	SetValueMethod = GetMethod("setValue", "(Ljava/lang/Double;)V");
	GetValueMethod = GetMethod("getValue", "()Ljava/lang/Double;");
	SetUnitMethod = GetMethod("setUnit", "(Ljava/lang/String;)V");
	GetUnitMethod = GetMethod("getUnit", "()Ljava/lang/String;");
}

void FAndroidSentryMetric::SetName(const FString& name)
{
	CallMethod<void>(SetNameMethod, *GetJString(name));
}

FString FAndroidSentryMetric::GetName() const
{
	return CallMethod<FString>(GetNameMethod);
}

void FAndroidSentryMetric::SetType(const FString& type)
{
	CallMethod<void>(SetTypeMethod, *GetJString(type));
}

FString FAndroidSentryMetric::GetType() const
{
	return CallMethod<FString>(GetTypeMethod);
}

void FAndroidSentryMetric::SetValue(float value)
{
	TSharedPtr<FSentryJavaObjectWrapper> javaDouble = MakeShareable(new FSentryJavaObjectWrapper(SentryJavaClasses::Double, "(D)V", static_cast<double>(value)));
	CallMethod<void>(SetValueMethod, javaDouble->GetJObject());
}

float FAndroidSentryMetric::GetValue() const
{
	auto valueObject = CallObjectMethod<jobject>(GetValueMethod);
	if (!valueObject)
	{
		return 0.0f;
	}

	FSentryJavaObjectWrapper valueWrapper(SentryJavaClasses::Double, *valueObject);
	FSentryJavaMethod floatValueMethod = valueWrapper.GetMethod("floatValue", "()F");
	return valueWrapper.CallMethod<float>(floatValueMethod);
}

void FAndroidSentryMetric::SetUnit(const FString& unit)
{
	CallMethod<void>(SetUnitMethod, *GetJString(unit));
}

FString FAndroidSentryMetric::GetUnit() const
{
	return CallMethod<FString>(GetUnitMethod);
}

void FAndroidSentryMetric::SetAttribute(const FString& key, const FSentryVariant& value)
{
	TSharedPtr<FSentryJavaObjectWrapper> attribute = FAndroidSentryConverters::VariantToNative(value);

	if (!attribute)
	{
		return;
	}

	CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "setMetricAttribute", "(Lio/sentry/SentryMetricsEvent;Ljava/lang/String;Ljava/lang/Object;)V",
		GetJObject(), *GetJString(key), attribute->GetJObject());
}

FSentryVariant FAndroidSentryMetric::GetAttribute(const FString& key) const
{
	auto attribute = CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "getMetricAttribute", "(Lio/sentry/SentryMetricsEvent;Ljava/lang/String;)Ljava/lang/Object;",
		GetJObject(), *GetJString(key));

	if (!attribute)
	{
		return FSentryVariant();
	}

	return FAndroidSentryConverters::VariantToUnreal(*attribute);
}

bool FAndroidSentryMetric::TryGetAttribute(const FString& key, FSentryVariant& value) const
{
	auto attribute = CallStaticObjectMethod<jobject>(SentryJavaClasses::SentryBridgeJava, "getMetricAttribute", "(Lio/sentry/SentryMetricsEvent;Ljava/lang/String;)Ljava/lang/Object;",
		GetJObject(), *GetJString(key));

	if (!attribute)
	{
		return false;
	}

	value = FAndroidSentryConverters::VariantToUnreal(*attribute);

	return true;
}

void FAndroidSentryMetric::RemoveAttribute(const FString& key)
{
	CallStaticMethod<void>(SentryJavaClasses::SentryBridgeJava, "removeMetricAttribute", "(Lio/sentry/SentryMetricsEvent;Ljava/lang/String;)V",
		GetJObject(), *GetJString(key));
}

void FAndroidSentryMetric::AddAttributes(const TMap<FString, FSentryVariant>& attributes)
{
	for (const auto& pair : attributes)
	{
		SetAttribute(pair.Key, pair.Value);
	}
}
