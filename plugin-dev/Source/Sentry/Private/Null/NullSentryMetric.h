// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryMetricInterface.h"

class FNullSentryMetric : public ISentryMetric
{
public:
	FNullSentryMetric() {}
	virtual ~FNullSentryMetric() override = default;

	virtual void SetName(const FString& name) override {}
	virtual FString GetName() const override { return FString(); }
	virtual void SetType(const FString& type) override {}
	virtual FString GetType() const override { return FString(); }
	virtual void SetValue(double value) override {}
	virtual double GetValue() const override { return 0.0; }
	virtual void SetUnit(const FString& unit) override {}
	virtual FString GetUnit() const override { return FString(); }

	virtual void SetAttribute(const FString& key, const FSentryVariant& value) override {}
	virtual FSentryVariant GetAttribute(const FString& key) const override { return FSentryVariant(); }
	virtual bool TryGetAttribute(const FString& key, FSentryVariant& value) const override { return false; }
	virtual void RemoveAttribute(const FString& key) override {}
	virtual void AddAttributes(const TMap<FString, FSentryVariant>& attributes) override {}
};

typedef FNullSentryMetric FPlatformSentryMetric;
