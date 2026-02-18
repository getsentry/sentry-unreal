// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

class USentryBeforeSendHandler;
class USentryBeforeBreadcrumbHandler;
class USentryBeforeLogHandler;
class USentryBeforeMetricHandler;
class USentryTraceSampler;

/**
 * Groups all callback handler pointers passed to ISentrySubsystem::InitWithSettings().
 * Adding a new callback means adding a new field here (defaulting to nullptr),
 * without changing InitWithSettings signatures across platform implementations.
 */
struct FSentryCallbackHandlers
{
	USentryBeforeSendHandler* BeforeSendHandler = nullptr;
	USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler = nullptr;
	USentryBeforeLogHandler* BeforeLogHandler = nullptr;
	USentryBeforeMetricHandler* BeforeMetricHandler = nullptr;
	USentryTraceSampler* TraceSampler = nullptr;
};
