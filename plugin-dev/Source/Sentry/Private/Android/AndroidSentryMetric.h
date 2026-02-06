// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryMetricInterface.h"

#include "Infrastructure/AndroidSentryJavaObjectWrapper.h"

class FAndroidSentryMetric : public ISentryMetric, public FSentryJavaObjectWrapper
{
public:
	FAndroidSentryMetric();
	FAndroidSentryMetric(jobject metricEvent);

	void SetupClassMethods();

	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetType(const FString& type) override;
	virtual FString GetType() const override;
	virtual void SetValue(double value) override;
	virtual double GetValue() const override;
	virtual void SetUnit(const FString& unit) override;
	virtual FString GetUnit() const override;

	virtual void SetAttribute(const FString& key, const FSentryVariant& value) override;
	virtual FSentryVariant GetAttribute(const FString& key) const override;
	virtual bool TryGetAttribute(const FString& key, FSentryVariant& value) const override;
	virtual void RemoveAttribute(const FString& key) override;
	virtual void AddAttributes(const TMap<FString, FSentryVariant>& attributes) override;

private:
	FSentryJavaMethod SetNameMethod;
	FSentryJavaMethod GetNameMethod;
	FSentryJavaMethod SetTypeMethod;
	FSentryJavaMethod GetTypeMethod;
	FSentryJavaMethod SetValueMethod;
	FSentryJavaMethod GetValueMethod;
	FSentryJavaMethod SetUnitMethod;
	FSentryJavaMethod GetUnitMethod;
};

typedef FAndroidSentryMetric FPlatformSentryMetric;
