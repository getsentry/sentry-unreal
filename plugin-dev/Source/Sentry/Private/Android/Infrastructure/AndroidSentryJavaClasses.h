// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "AndroidSentryDataTypes.h"

struct SentryJavaClasses
{
	// External Java classes
	const static FSentryJavaClass SentryBridgeJava;
	const static FSentryJavaClass Sentry;
	const static FSentryJavaClass Attachment;
	const static FSentryJavaClass Breadcrumb;
	const static FSentryJavaClass SentryEvent;
	const static FSentryJavaClass SentryId;
	const static FSentryJavaClass Scope;
	const static FSentryJavaClass ScopeImpl;
	const static FSentryJavaClass User;
	const static FSentryJavaClass UserFeedback;
	const static FSentryJavaClass Message;
	const static FSentryJavaClass SentryLevel;
	const static FSentryJavaClass SentryHint;
	const static FSentryJavaClass Transaction;
	const static FSentryJavaClass Span;
	const static FSentryJavaClass SamplingContext;
	const static FSentryJavaClass CustomSamplingContext;
	const static FSentryJavaClass TransactionContext;
	const static FSentryJavaClass TransactionOptions;
	const static FSentryJavaClass SentryTraceHeader;

	// System Java classes
	const static FSentryJavaClass ArrayList;
	const static FSentryJavaClass HashMap;
	const static FSentryJavaClass Map;
	const static FSentryJavaClass Set;
	const static FSentryJavaClass Iterator;
	const static FSentryJavaClass MapEntry;
	const static FSentryJavaClass List;
	const static FSentryJavaClass Double;
	const static FSentryJavaClass Integer;
	const static FSentryJavaClass Float;
	const static FSentryJavaClass Boolean;
	const static FSentryJavaClass String;
};