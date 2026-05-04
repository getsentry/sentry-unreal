// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#if !USE_SENTRY_NATIVE

#include "Interface/SentryMetricInterface.h"

@class SentryObjCMetric;

class FAppleSentryMetric : public ISentryMetric
{
public:
	FAppleSentryMetric();
	FAppleSentryMetric(SentryObjCMetric* metric);
	virtual ~FAppleSentryMetric() override;

	SentryObjCMetric* GetNativeObject();

	virtual void SetName(const FString& name) override;
	virtual FString GetName() const override;
	virtual void SetType(const FString& type) override;
	virtual FString GetType() const override;
	virtual void SetValue(float value) override;
	virtual float GetValue() const override;
	virtual void SetUnit(const FString& unit) override;
	virtual FString GetUnit() const override;

	virtual void SetAttribute(const FString& key, const FSentryVariant& value) override;
	virtual FSentryVariant GetAttribute(const FString& key) const override;
	virtual bool TryGetAttribute(const FString& key, FSentryVariant& value) const override;
	virtual void RemoveAttribute(const FString& key) override;
	virtual void AddAttributes(const TMap<FString, FSentryVariant>& attributes) override;

private:
	SentryObjCMetric* MetricApple;
};

typedef FAppleSentryMetric FPlatformSentryMetric;

#endif // !USE_SENTRY_NATIVE
