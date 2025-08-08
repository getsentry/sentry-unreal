// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "UObject/GCObject.h"

// Custom callback class to trigger Sentry event capture during GC
class FSentryGCCallback : public FGCObject
{
public:
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
};