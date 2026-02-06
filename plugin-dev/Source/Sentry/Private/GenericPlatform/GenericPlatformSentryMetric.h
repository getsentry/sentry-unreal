// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryMetricInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryMetric : public ISentryMetric
{
public:
	FGenericPlatformSentryMetric();
	FGenericPlatformSentryMetric(sentry_value_t metric);
	virtual ~FGenericPlatformSentryMetric() override = default;

	sentry_value_t GetNativeObject();

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
	sentry_value_t Metric;
};

typedef FGenericPlatformSentryMetric FPlatformSentryMetric;

#endif // USE_SENTRY_NATIVE
